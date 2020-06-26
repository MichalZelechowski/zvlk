/* 
 * File:   Vulkan.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 11 czerwca 2020, 21:16
 */

#ifndef VULKAN_H
#define VULKAN_H

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <memory>

#include "Window.h"
#include "Device.h"
#include "Frame.h"

namespace zvlk {

    class DeviceAssessment {
    public:
        virtual int assess(zvlk::Device* device) = 0;
    };

    class Vulkan {
    public:
        Vulkan() = delete;
        Vulkan(bool debug, std::string applicationName);
        Vulkan(const Vulkan& orig) = delete;
        virtual ~Vulkan();

        void addSurface(zvlk::Window* window);
        zvlk::Device* getDevice(zvlk::DeviceAssessment* assessment);
        zvlk::Frame* initializeDeviceForGraphics(zvlk::Device* device);
        bool doesDeviceSupportExtensions(zvlk::Device* device);
        bool doesDeviceSupportGraphics(zvlk::Device* device);
        zvlk::SwapChainSupportDetails querySwapChainSupport(zvlk::Device* device);

        vk::SurfaceKHR surface;
        vk::Instance instance;
    private:
        std::vector<zvlk::Device*> devices;
        bool debug;
        vk::DebugUtilsMessengerEXT debugMessenger;

        const std::vector<const char*> validationLayers = {
            //    "VK_LAYER_KHRONOS_validation"
            "VK_LAYER_LUNARG_standard_validation"
        };
        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };


        bool checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData);

    };


}
#endif /* VULKAN_H */

