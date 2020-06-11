/* 
 * File:   Device.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 11 czerwca 2020, 23:17
 */

#ifndef DEVICE_H
#define DEVICE_H
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>

namespace zvlk {

    class Device {
    public:
        Device() = delete;
        Device(const Device& orig) = delete;

        Device(VkPhysicalDevice physicalDevice);
        virtual ~Device();
    private:
        VkPhysicalDevice physicalDevice;
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        VkPhysicalDeviceMemoryProperties memoryProperties;

    };
}
#endif /* DEVICE_H */
