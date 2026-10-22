#include "sensor.hpp"

#include <iostream>

namespace telemetry {

Sensor::Sensor(std::string name, EventBus& bus)
    : name_(std::move(name)), bus_(bus) {
    bus_.subscribe(this);
}

Sensor::~Sensor() {
    // Correct cleanup: a sensor removes itself from the bus so the bus never
    // holds a dangling pointer to it.
    bus_.unsubscribe(this);
}

void Sensor::on_event(const Event& ev) {
    last_value_ = ev.value;
    ++events_seen_;
}

}  // namespace telemetry
