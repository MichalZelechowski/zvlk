/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Vulkan.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 11 czerwca 2020, 21:16
 */

#ifndef VULKAN_H
#define VULKAN_H
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>
#include <string>

#include "Window.h"

namespace zvlk {

    class Vulkan {
    public:
        Vulkan() = delete;
        Vulkan(bool debug, std::string applicationName);
        Vulkan(const Vulkan& orig) = delete;
        virtual ~Vulkan();

        void addSurface(zvlk::Window* window);

        VkSurfaceKHR surface = nullptr;
        VkInstance instance;
    private:

        bool debug;
        VkDebugUtilsMessengerEXT debugMessenger;

        const std::vector<const char*> validationLayers = {
            //    "VK_LAYER_KHRONOS_validation"
            "VK_LAYER_LUNARG_standard_validation"
        };

        bool checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);
        VkResult createDebugUtilsMessenger(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo);

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData);

    };
}
#endif /* VULKAN_H */

