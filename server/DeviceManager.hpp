#pragma once

#include <string>
#include <map>
#include "SyncQueue.hpp"

struct Device {
    int id;
    SyncQueue* queue;
};

class DeviceManager {
    private:
        int deviceId = 1;
        std::string username;
        std::map<int, Device> devices;
        std::mutex lock;       
    public:
        DeviceManager(std::string username);
        Device& registerDevice();
        Device& getDevice(int deviceId);
        void disconnectDevice(int deviceId);
        void notifyAllDevices(FileOperation op);
        void notifyOtherDevices(FileOperation op, int sourceDeviceId);
};
