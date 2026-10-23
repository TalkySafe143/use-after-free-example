#include <iostream>
#include <vector>

#include "aggregator.hpp"
#include "device_manager.hpp"
#include "event_bus.hpp"
#include "sensor.hpp"

using namespace telemetry;

namespace {

// Simulate one round of incoming telemetry for the given sources.
void pump_events(EventBus &bus, const std::vector<std::string> &sources,
                 std::uint64_t &seq) {
  double base = static_cast<double>(seq) * 1.5;
  for (const auto &src : sources) {
    Event ev{src, seq++, base + 10.0};
    bus.publish(ev);
  }
}

} // namespace

int main() {
  EventBus bus;
  DeviceManager devices(bus);

  // An aggregator that tracks a rolling mean over a group of sensors.
  Aggregator power_group("power-group", bus);

  // --- Phase 1: initial configuration -------------------------------------
  // Three sensors come online and the aggregator is told to watch them.
  Sensor *s_a = devices.add_sensor("inverter-A");
  Sensor *s_b = devices.add_sensor("inverter-B");
  Sensor *s_c = devices.add_sensor("inverter-C");

  power_group.watch(s_a);
  power_group.watch(s_b);
  power_group.watch(s_c);

  std::uint64_t seq = 0;
  std::vector<std::string> active = {"inverter-A", "inverter-B", "inverter-C"};

  std::cout << "[phase 1] running with " << devices.size() << " sensors\n";
  for (int round = 0; round < 3; ++round) {
    pump_events(bus, active, seq);
    std::cout << "  round " << round << " mean=" << power_group.last_mean()
              << "\n";
  }

  // --- Phase 2: hot reconfiguration ---------------------------------------
  // Operations decides inverter-B is being taken offline for maintenance, so
  // it is removed from the device manager. This frees the Sensor object.
  //
  // BUG: whoever wrote this reconfiguration path updated `devices` and
  // the `active` source list, but forgot that `power_group` still has a
  // cached pointer to inverter-B. The aggregator is never told to stop
  // watching it.
  std::cout << "[phase 2] taking inverter-B offline for maintenance\n";
  devices.remove_sensor("inverter-B");
  active = {"inverter-A", "inverter-C"};

  std::cout << "[phase 2] running with " << devices.size() << " sensors\n";
  for (int round = 0; round < 3; ++round) {
    // On the first publish here, the aggregator's on_event() iterates its
    // watched list and dereferences the freed inverter-B pointer.
    pump_events(bus, active, seq);
    std::cout << "  round " << round << " mean=" << power_group.last_mean()
              << "\n";
  }

  std::cout << "done\n";
  return 0;
}
