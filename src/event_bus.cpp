#include "event_bus.hpp"

#include <algorithm>
#include <iostream>

namespace telemetry {

void EventBus::subscribe(IObserver* obs) {
    if (!obs) return;
    observers_.push_back(obs);
}

void EventBus::unsubscribe(IObserver* obs) {
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), obs),
        observers_.end());
}

void EventBus::publish(const Event& ev) {
    // Snapshot not taken on purpose: in this design observers are not
    // supposed to mutate the subscription list during dispatch.
    for (IObserver* obs : observers_) {
        // If `obs` points at a destroyed object, THIS is the line that
        // dereferences freed memory. The call looks completely innocent.
        obs->on_event(ev);
    }
}

}  // namespace telemetry
