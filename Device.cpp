/* 
 * File:   Device.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 11 czerwca 2020, 23:17
 */

#include "Device.h"

namespace zvlk {

    Device::Device(VkPhysicalDevice physicalDevice) {
        this->physicalDevice = physicalDevice;

        vkGetPhysicalDeviceProperties(this->physicalDevice, &this->deviceProperties);
        vkGetPhysicalDeviceFeatures(this->physicalDevice, &this->deviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memoryProperties);

        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());
    }

    Device::~Device() {
    }

}