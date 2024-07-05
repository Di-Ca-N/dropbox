#include "DeviceManager.hpp"
#include <mutex>


DeviceManager::DeviceManager(std::string username) {
    this->username = username;
}

Device& DeviceManager::registerDevice() {
    // Locking to ensure that every device has a unique id
    lock.lock();
    Device device = {
        .id=deviceId,
        .queue=new SyncQueue(),
    };
    int id = deviceId;
    devices[id] = device;
    deviceId++;
    lock.unlock(); // After generating and saving the device, we can unlock the mutex
    return devices[id];
}

Device& DeviceManager::getDevice(int deviceId) { 
    return devices[deviceId]; 
}

void DeviceManager::disconnectDevice(int deviceId) {
    // Locking to ensure consistency when deleting a device
    lock.lock();
    Device device = devices[deviceId];
    delete device.queue;
    devices.erase(deviceId);
    lock.unlock();
}

void DeviceManager::notifyAllDevices(FileOperation op) {
    for (auto &[id, device] : devices) {
        device.queue->push(op);
    }
}

void DeviceManager::notifyOtherDevices(FileOperation op, int sourceDeviceId) {
    for (auto &[id, device] : devices) {
        if (id != sourceDeviceId) {
            device.queue->push(op);
        }
    } 
}
