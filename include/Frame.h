/* 
 * File:   Frame.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 16 czerwca 2020, 22:57
 */

#ifndef FRAME_H
#define FRAME_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>
#include <array>

#include "Device.h"

namespace zvlk {

    class Device;
    struct SwapChainSupportDetails;

    class Frame {
    public:
        Frame() = delete;
        Frame(zvlk::Device* device, VkSurfaceKHR& surface);
        Frame(const Frame& orig) = delete;
        virtual ~Frame();

        zvlk::SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface, const VkDevice graphicsDevice);
        uint32_t getImagesNumber();
        uint32_t getWidth();
        uint32_t getHeight();
        VkRenderPass getRenderPass();

        inline VkSwapchainKHR getShwapChain() {
            return this->swapChain;
        };
        
        VkRenderPassBeginInfo getRenderPassBeginInfo(uint32_t index);
    private:
        VkDevice graphicsDevice;

        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;

        VkRenderPass renderPass;
        VkImage colorImage;
        VkDeviceMemory colorImageMemory;
        VkImageView colorImageView;
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
        std::vector<VkFramebuffer> swapChainFramebuffers;
        std::vector<VkClearValue> clearValues;

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities);
        VkFormat findDepthFormat(zvlk::Device* device);

    };
}
#endif /* FRAME_H */

