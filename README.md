# Telemetry demo — use-after-free caught by AddressSanitizer

A small-but-realistic C++17 project built to demonstrate a **non-trivial
use-after-free (UAF)** and how **AddressSanitizer (ASan)** turns a silent,
scary bug into a precise, actionable report.

## The scenario

A telemetry system with four cooperating components:

| Component       | Role                                                              | Owns sensors? |
|-----------------|-------------------------------------------------------------------|---------------|
| `EventBus`      | Dispatches events to observers via **non-owning** raw pointers.   | no            |
| `Sensor`        | Observer with RAII: subscribes in ctor, unsubscribes in dtor.     | —             |
| `Aggregator`    | Observer that **caches `Sensor*`** to compute a rolling mean.     | no            |
| `DeviceManager` | **Owns** sensors via `unique_ptr`; supports hot reconfiguration.  | yes           |

Note that each class, read on its own, looks correct:

- `EventBus` documents that it does not own observers.
- `Sensor` follows textbook RAII and cleans up after itself.
- `DeviceManager` owns sensors and frees them properly.

## The bug

The defect is **emergent** — it only exists in the *interaction*:

1. `Aggregator::watch()` caches raw `Sensor*` pointers but never tracks their
   lifetime (`include/aggregator.hpp`).
2. During "hot reconfiguration" in `main.cpp` (phase 2), a sensor is removed
   from `DeviceManager`, which **frees** it.
3. The reconfiguration code forgets to tell the `Aggregator` to stop watching
   that sensor.
4. The next `EventBus::publish()` calls `Aggregator::on_event()`, which reads
   `s->last_value()` through the **dangling pointer** → use-after-free.

The free happens in `device_manager.cpp`, but the crash (if any) surfaces in
`aggregator.cpp`, in a *different* event cycle. That distance in both space and
time is exactly what makes real UAF bugs hard to find by reading code.

## Why ASan matters here

Build and run the plain binary:

```bash
make run
```

It runs to completion and prints **plausible numbers** — the UAF reads
freed-but-still-mapped memory and no crash occurs. This is the dangerous case:
the program is already broken but *looks* fine. In production it can corrupt
data, crash intermittently, or become a security vulnerability.

Now build and run the ASan binary:

```bash
make run-asan
```

ASan reports immediately and precisely:

```
ERROR: AddressSanitizer: heap-use-after-free ...
READ of size 8 ...
  #0 telemetry::Sensor::last_value() const   include/sensor.hpp:25
  #1 telemetry::Aggregator::on_event(...)    src/aggregator.cpp:28   <- where it blew up
  #2 telemetry::EventBus::publish(...)        src/event_bus.cpp:25
  ...
freed by thread T0 here:
  #1 telemetry::Sensor::~Sensor()            src/sensor.cpp:16
  ...
  #12 telemetry::DeviceManager::remove_sensor(...) src/device_manager.cpp:18  <- where it was freed
previously allocated by thread T0 here:
  #2 telemetry::DeviceManager::add_sensor(...)     src/device_manager.cpp:6   <- where it was born
```

In one shot you get the three facts you need to fix a UAF:
- **where the bad access happened** (aggregator reading a sensor),
- **where the memory was freed** (`remove_sensor`),
- **where it was originally allocated** (`add_sensor`).

## Build targets

```bash
make            # plain -O2 build  -> build/telemetry      (bug is silent)
make asan       # ASan -O1 build   -> build/telemetry_asan (bug is reported)
make run        # build + run plain
make run-asan   # build + run ASan (this is the demo payoff)
make clean
```

## Requirements

- g++ with ASan support (tested on g++ 16).
- The ASan **runtime** must be installed. On Fedora/RHEL:

  ```bash
  sudo dnf install -y libasan
  ```

  (Debian/Ubuntu ship it with `g++`; on those systems no extra install is
  usually needed.)

## The fix (left out on purpose)

The project intentionally ships with the bug. The teaching exercise is to use
the ASan report to find and fix it. The clean fix is to break the lifetime
coupling — for example:

- give `Aggregator` an `unwatch(const Sensor*)` and call it from the
  reconfiguration path, or
- have `Aggregator` hold `std::weak_ptr<Sensor>` / observe sensor destruction,
  so a freed sensor can never be dereferenced.
