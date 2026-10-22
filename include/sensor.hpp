#pragma once

#include <string>

#include "event_bus.hpp"

namespace telemetry {

// A sensor is an observer that keeps a running state based on the events it
// sees. It subscribes itself to the bus on construction and unsubscribes on
// destruction -- the textbook RAII approach, which on its own is correct.
class Sensor : public IObserver {
public:
    Sensor(std::string name, EventBus& bus);
    ~Sensor() override;

    // Non-copyable, non-movable: its address is registered with the bus, so
    // moving it would invalidate the registered pointer.
    Sensor(const Sensor&) = delete;
    Sensor& operator=(const Sensor&) = delete;

    void on_event(const Event& ev) override;
    const std::string& name() const override { return name_; }

    double last_value() const { return last_value_; }
    std::uint64_t events_seen() const { return events_seen_; }

private:
    std::string name_;
    EventBus& bus_;
    double last_value_ = 0.0;
    std::uint64_t events_seen_ = 0;
};

}  // namespace telemetry
