/* 
 * File:   Vulkan.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 11 czerwca 2020, 21:16
 */

#include "Vulkan.h"
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <algorithm>


namespace zvlk {

    Vulkan::Vulkan(bool debug, std::string applicationName) {
        this->debug = debug;
        if (this->debug && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        auto extensions = getRequiredExtensions();

        vk::ApplicationInfo appInfo(applicationName.data(), VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);
        vk::InstanceCreateInfo createInfo(vk::InstanceCreateFlags(), &appInfo,
                debug ? static_cast<uint32_t> (validationLayers.size()) : 0,
                debug ? validationLayers.data() : nullptr,
                static_cast<uint32_t> (extensions.size()),
                extensions.data());

        if (debug) {
            vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo({},
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
                    debugCallback);
            createInfo.setPNext(&debugCreateInfo);

            this->instance = vk::createInstance(createInfo);

            vk::DispatchLoaderDynamic dldy;
            dldy.init(instance);

            this->debugMessenger = this->instance.createDebugUtilsMessengerEXT(debugCreateInfo, nullptr, dldy);
        } else {
            this->instance = vk::createInstance(createInfo);
        }

        std::vector<vk::PhysicalDevice> physicalDevices = this->instance.enumeratePhysicalDevices();
        for (VkPhysicalDevice pD : physicalDevices) {
            this->devices.push_back(new Device(pD));
        }
    }

    Vulkan::~Vulkan() {
        for (Device* device : this->devices) {
            delete device;
        }

        this->destroySurface();
        
        if (this->debug) {
            vk::DispatchLoaderDynamic dldy;
            dldy.init(this->instance);
            this->instance.destroyDebugUtilsMessengerEXT(this->debugMessenger, nullptr, dldy);
        }

        this->instance.destroy();
    }

    void Vulkan::destroySurface() {
        if (this->surface) {
            this->instance.destroySurfaceKHR(this->surface);
            this->surface = vk::SurfaceKHR();
        }
    }

    void Vulkan::addSurface(zvlk::Window* window) {
        VkSurfaceKHR surfaceClassic;
        if (this->surface || glfwCreateWindowSurface(this->instance, window->getWindow(), nullptr, &surfaceClassic) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        this->surface = surfaceClassic;
    }

    bool Vulkan::checkValidationLayerSupport() {
        std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

        for (const char* layerName : this->validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                std::cout << layerProperties.layerName << std::endl;
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> Vulkan::getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (this->debug) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        std::cout << "Extensions: " << std::endl;
        for (auto& extension : extensions) {
            std::cout << extension << std::endl;
        }

        return extensions;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) {

        std::cerr << std::hex << messageSeverity;
        std::cerr << " Validation layer: ";
        std::cerr << std::hex << messageType;
        std::cerr << " [" << pCallbackData->pMessageIdName << "] " << pCallbackData->pMessage << std::endl;
        for (uint32_t i = 0; i < pCallbackData->objectCount; ++i) {
            const char* objectName = pCallbackData->pObjects[i].pObjectName == nullptr ? "" : pCallbackData->pObjects[i].pObjectName;
            std::cerr << i << " object " << pCallbackData->pObjects[i].objectType;
            std::cerr << " " << objectName << std::endl;
        }

        return VK_FALSE;
    }

    Device* Vulkan::getDevice(DeviceAssessment* assessment) {
        std::vector<int> scores;

        for (Device* device : this->devices) {
            scores.push_back(assessment->assess(device));
        }

        Device* result = this->devices[std::distance(scores.begin(), std::max_element(scores.begin(), scores.end()))];
        return result;
    }

    Frame* Vulkan::initializeDeviceForGraphics(zvlk::Device* device) {
        const std::vector<const char*> noValidations;
        zvlk::Frame* result = device->initializeForGraphics(this->surface, this->debug ? validationLayers : noValidations, deviceExtensions);
        return result;
    }

    bool Vulkan::doesDeviceSupportExtensions(zvlk::Device* device) {
        return device->doesSupportExtensions(this->deviceExtensions);
    }

    bool Vulkan::doesDeviceSupportGraphics(zvlk::Device* device) {
        return device->doesSupportGraphics(this->surface);
    }

    zvlk::SwapChainSupportDetails Vulkan::querySwapChainSupport(zvlk::Device* device) {
        return device->querySwapChainSupport(this->surface);
    }

}