#include "aggregator.hpp"

namespace telemetry {

Aggregator::Aggregator(std::string name, EventBus& bus)
    : name_(std::move(name)), bus_(bus) {
    bus_.subscribe(this);
}

Aggregator::~Aggregator() {
    bus_.unsubscribe(this);
}

void Aggregator::watch(const Sensor* s) {
    watched_.push_back(s);
}

void Aggregator::on_event(const Event& /*ev*/) {
    if (watched_.empty()) return;

    double sum = 0.0;
    for (const Sensor* s : watched_) {
        // s->last_value() reads through the cached pointer. If that sensor has
        // since been destroyed, this is a use-after-free -- but the crash, if
        // any, surfaces HERE, far from where the sensor was actually freed.
        sum += s->last_value();
    }
    last_mean_ = sum / static_cast<double>(watched_.size());
}

}  // namespace telemetry
