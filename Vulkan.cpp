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

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = applicationName.data();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (debug) {
            createInfo.enabledLayerCount = static_cast<uint32_t> (validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
            this->populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t> (extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkResult result = vkCreateInstance(&createInfo, nullptr, &this->instance);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create vulkan instance!");
        }

        if (debug && this->createDebugUtilsMessenger(const_cast<VkDebugUtilsMessengerCreateInfoEXT*> (&debugCreateInfo)) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(this->instance, &deviceCount, physicalDevices.data());
        this->devices.resize(deviceCount);
        for (VkPhysicalDevice pD : physicalDevices) {
            this->devices.push_back(new Device(pD));
        }
    }

    Vulkan::~Vulkan() {
        for (Device* device : this->devices) {
            delete device;
        }

        if (this->surface != nullptr) {
            vkDestroySurfaceKHR(this->instance, surface, nullptr);
        }

        if (this->debug) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr) {
                func(this->instance, this->debugMessenger, nullptr);
            }
        }

        vkDestroyInstance(instance, nullptr);
    }

    void Vulkan::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo) {
        debugCreateInfo = {};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        debugCreateInfo.pUserData = nullptr;
    }

    VkResult Vulkan::createDebugUtilsMessenger(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(this->instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(this->instance, pCreateInfo, nullptr, &this->debugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void Vulkan::addSurface(zvlk::Window* window) {
        //TODO think about vector
        if (this->surface != nullptr || glfwCreateWindowSurface(this->instance, window->getWindow(), nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    bool Vulkan::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

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
        for (int i = 0; i < pCallbackData->objectCount; ++i) {
            const char* objectName = pCallbackData->pObjects[i].pObjectName == nullptr ? "" : pCallbackData->pObjects[i].pObjectName;
            std::cerr << i << " object " << pCallbackData->pObjects[i].objectType;
            std::cerr << " " << objectName << std::endl;
        }

        return VK_FALSE;
    }

    Device* Vulkan::getDevice(DeviceAssessment* assessment) {
        std::vector<int> scores(this->devices.size());

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