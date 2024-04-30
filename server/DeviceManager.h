#include <string>
#include <map>
#include "SyncQueue.h"

struct Device {
    int id;
    SyncQueue* queue;
};

class DeviceManager {
    private:
        static int deviceId;
        std::string username;
        std::map<int, Device> devices;
        std::mutex lock;       
    public:
        DeviceManager(std::string username);
        Device registerDevice();
        void disconnectDevice(int deviceId);
        void notifyAllDevices(FileOperation op);
        void notifyOtherDevices(FileOperation op, int sourceDeviceId);
};