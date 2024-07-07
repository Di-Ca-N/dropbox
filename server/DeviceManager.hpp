#pragma once

#include <string>
#include <map>
#include "SyncQueue.hpp"

struct Device {
    int id;
    int numConnections = 0;
    SyncQueue* queue;
};

// Manage devices of a single user
class DeviceManager {
    private:
        int deviceId = 1; // Next deviceId to be assigned. Must start at 1.
        std::string username;
        std::map<int, Device> devices;
        std::mutex mutex;   
    public:
        DeviceManager(std::string username);
        // Register a new device for the user and returns a reference to it. The returned device is 
        // guaranteed to have a unique ID for that user.
        Device& registerDevice();
        // Returns the device with the given ID.
        Device getDevice(int deviceId);
        // Register a new connection for the device with the given ID.  
        void connectDevice(int deviceId);
        // Disconnects one connection for the device with the given ID. If the number of device connections reaches 0,
        // it will be deleted.
        void disconnectDevice(int deviceId);
        // Notify all devices of the same user about the FileOperation
        void notifyAllDevices(FileOperation op);
        // Notify all devices of the same user about the FileOperation, except the device that 
        // created the operation that notification.
        void notifyOtherDevices(FileOperation op, int sourceDeviceId);
};
