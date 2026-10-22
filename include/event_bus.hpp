#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace telemetry {

// A single telemetry reading pushed through the system.
struct Event {
    std::string source;   // logical name of the producer
    std::uint64_t seq;    // monotonically increasing sequence id
    double value;         // the measured value
};

// Anything that wants to receive events implements this interface.
// NOTE: the bus stores *non-owning* pointers to observers. Observers are
// responsible for unsubscribing themselves before they are destroyed.
class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void on_event(const Event& ev) = 0;
    // Human-readable id, used only for logging/diagnostics.
    virtual const std::string& name() const = 0;
};

// Central dispatcher. Observers register here and get called back on publish().
// The bus deliberately keeps raw pointers: it does not own the observers and
// does not extend their lifetime. This mirrors a very common real-world
// pattern (Qt signals, GObject, many C-style callback registries).
class EventBus {
public:
    void subscribe(IObserver* obs);
    void unsubscribe(IObserver* obs);

    // Fan out a single event to every currently-subscribed observer.
    void publish(const Event& ev);

    std::size_t observer_count() const { return observers_.size(); }

private:
    std::vector<IObserver*> observers_;
};

}  // namespace telemetry
