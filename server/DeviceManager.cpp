#include "DeviceManager.hpp"
#include <iostream>

DeviceManager::DeviceManager(std::string username, int maxDevices) {
    this->username = username;
    this->maxDevices = maxDevices;
}

Device& DeviceManager::registerDevice() {
    // Locking to ensure consistency when registering a device
    std::lock_guard<std::mutex> lock(mutex);

    if (maxDevices >= 0 and devices.size() >= maxDevices) {
        throw TooManyDevices(maxDevices);
    }

    Device device = {
        .id=deviceId,
        .numConnections=0,
        .queue=new SyncQueue(),
    };
    int id = deviceId;
    devices[id] = device;
    deviceId++;
    return devices[id];
}

Device DeviceManager::getDevice(int deviceId) {
    // Locking to ensure that the device does not change during read
    std::lock_guard<std::mutex> lock(mutex);
    return devices[deviceId];
}

void DeviceManager::connectDevice(int deviceId) {
    // Locking to ensure consistency when adding a device connection
    std::lock_guard<std::mutex> lock(mutex);
    Device& device = devices[deviceId];
    device.numConnections++;
}

void DeviceManager::disconnectDevice(int deviceId) {
    // Locking to ensure consistency when disconnecting a device
    std::lock_guard<std::mutex> lock(mutex);
    Device& device = devices[deviceId];

    if (device.numConnections > 0) {
        device.numConnections--;
    }

    if (device.numConnections == 0) {
        std::cout << "Delete device with id " << device.id << "\n";
        delete device.queue;
        devices.erase(deviceId);
    }
}

void DeviceManager::notifyAllDevices(FileOperation op) {
    // Locking to avoid devices being added or removed during the notification
    std::lock_guard<std::mutex> lock(mutex);
    for (auto &[id, device] : devices) {
        device.queue->push(op);
    }
}

void DeviceManager::notifyOtherDevices(FileOperation op, int sourceDeviceId) {
    // Locking to avoid devices being added or removed during the notification
    std::lock_guard<std::mutex> lock(mutex); 
    for (auto &[id, device] : devices) {
        if (id != sourceDeviceId) {
            device.queue->push(op);
        }
    } 
}
