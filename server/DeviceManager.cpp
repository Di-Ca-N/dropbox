#include "DeviceManager.hpp"
#include <iostream>

DeviceManager::DeviceManager(std::string username) {
    this->username = username;
}

Device& DeviceManager::registerDevice() {
    std::lock_guard<std::mutex> lock(mutex);
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
    std::lock_guard<std::mutex> lock(mutex);
    return devices[deviceId];
}

void DeviceManager::connectDevice(int deviceId) {
    std::lock_guard<std::mutex> lock(mutex);
    Device& device = devices[deviceId];
    device.numConnections++;
}

void DeviceManager::disconnectDevice(int deviceId) {
    // Locking to ensure consistency when deleting a device
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
