
#include "DeviceManager.hpp"
#include <mutex>

int DeviceManager::deviceId = 0;

DeviceManager::DeviceManager(std::string username) {
    this->username = username;
}

Device DeviceManager::registerDevice() {
    lock.lock();
    Device device = {
        .id=deviceId,
        .queue=new SyncQueue(),
    };
    int id = deviceId;
    devices[id] = device;
    deviceId++;
    lock.unlock();
    return device;
}

void DeviceManager::disconnectDevice(int deviceId) {
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
