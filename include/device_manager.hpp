#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "event_bus.hpp"
#include "sensor.hpp"

namespace telemetry {

// Owns sensors and supports hot reconfiguration at runtime (sensors can be
// added and removed while the system keeps running). Ownership lives here via
// unique_ptr, so removing a sensor from the manager actually frees it.
class DeviceManager {
public:
    explicit DeviceManager(EventBus& bus) : bus_(bus) {}

    // Create and take ownership of a new sensor. Returns a borrowed pointer so
    // callers can wire it up (e.g. hand it to an Aggregator to watch).
    Sensor* add_sensor(const std::string& name);

    // Remove (and destroy) a sensor by name. This is where a sensor's lifetime
    // ends. The Sensor destructor unsubscribes it from the bus -- but it does
    // NOT, and cannot, tell any Aggregator that happens to be watching it.
    bool remove_sensor(const std::string& name);

    std::size_t size() const { return sensors_.size(); }

private:
    EventBus& bus_;
    std::unordered_map<std::string, std::unique_ptr<Sensor>> sensors_;
};

}  // namespace telemetry
