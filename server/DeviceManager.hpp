#pragma once

#include <string>
#include <map>
#include <stdexcept>
#include "SyncQueue.hpp"

struct Device {
    int id;
    int numConnections = 0;
    SyncQueue* queue;
};


class TooManyDevices : public std::exception {
   private:
    int maxDevices;
    std::string msg;
   public:
    TooManyDevices(int maxDevices) {
        this->maxDevices = maxDevices;
        msg = "User can only connect up to " + std::to_string(maxDevices) + " devices at the same time";
    };

    const char* what() const throw() { return msg.c_str(); }
};


// Manage devices of a single user
class DeviceManager {
    private:
        int deviceId = 1; // Next deviceId to be assigned. Must start at 1.
        int maxDevices; // Maximum number of devices that can be connected at the same time. Set to -1 to disable the limit
        std::string username;
        std::map<int, Device> devices;
        std::mutex mutex;   
    public:
        // DeviceManager for the given user. Set maxDevices to -1 to remove the limit
        DeviceManager(std::string username, int maxDevices=-1);
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
