#pragma once

#include <string>
#include <vector>

#include "event_bus.hpp"
#include "sensor.hpp"

namespace telemetry {

// The aggregator listens on the bus and, every time ANY event comes in, it
// recomputes a summary statistic (mean of the latest values) across a group of
// sensors it was told to watch.
//
// To read each sensor's current value it caches raw Sensor* pointers. This is
// the subtle design flaw: the aggregator's lifetime is decoupled from the
// sensors it points at, and nothing tells it when one of those sensors dies.
class Aggregator : public IObserver {
public:
    Aggregator(std::string name, EventBus& bus);
    ~Aggregator() override;

    Aggregator(const Aggregator&) = delete;
    Aggregator& operator=(const Aggregator&) = delete;

    // Register a sensor to be included in the aggregate.
    void watch(const Sensor* s);

    void on_event(const Event& ev) override;
    const std::string& name() const override { return name_; }

    double last_mean() const { return last_mean_; }

private:
    std::string name_;
    EventBus& bus_;
    std::vector<const Sensor*> watched_;  // non-owning, lifetime NOT tracked
    double last_mean_ = 0.0;
};

}  // namespace telemetry
