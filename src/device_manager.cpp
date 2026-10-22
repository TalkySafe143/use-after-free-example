#include "device_manager.hpp"

namespace telemetry {

Sensor* DeviceManager::add_sensor(const std::string& name) {
    auto sensor = std::make_unique<Sensor>(name, bus_);
    Sensor* raw = sensor.get();
    sensors_[name] = std::move(sensor);
    return raw;
}

bool DeviceManager::remove_sensor(const std::string& name) {
    auto it = sensors_.find(name);
    if (it == sensors_.end()) return false;
    // Erasing the unique_ptr runs ~Sensor(): the sensor unsubscribes from the
    // bus and its memory is freed. Any Aggregator still holding this pointer is
    // now holding a dangling pointer -- the latent bug is armed right here.
    sensors_.erase(it);
    return true;
}

}  // namespace telemetry
