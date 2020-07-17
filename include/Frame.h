/* 
 * File:   Frame.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 16 czerwca 2020, 22:57
 */

#ifndef FRAME_H
#define FRAME_H

#include <vulkan/vulkan.hpp>
#include <vector>
#include <array>

#include "Device.h"
#include "Window.h"

namespace zvlk {

    class Device;
    struct SwapChainSupportDetails;

    class Frame {
    public:
        Frame() = delete;
        Frame(zvlk::Device* device, vk::SurfaceKHR surface);
        Frame(const Frame& orig) = delete;
        virtual ~Frame();

        void create(zvlk::Device* device, vk::SurfaceKHR surface);
        void destroy();

        inline void attachWindow(std::shared_ptr<zvlk::Window> window) {
            this->window = window;
        }

        zvlk::SwapChainSupportDetails querySwapChainSupport(vk::SurfaceKHR surface, const vk::Device graphicsDevice);
        uint32_t getImagesNumber();
        uint32_t getWidth();
        uint32_t getHeight();
        vk::RenderPass getRenderPass();

        inline vk::SwapchainKHR getSwapChain() const {
            return this->swapChain;
        };

        vk::RenderPassBeginInfo getRenderPassBeginInfo(uint32_t index) const;
    private:
        std::shared_ptr<zvlk::Window> window;
        vk::Device graphicsDevice;

        vk::SwapchainKHR swapChain;
        std::vector<vk::Image> swapChainImages;
        vk::Format swapChainImageFormat;
        vk::Extent2D swapChainExtent;
        std::vector<vk::ImageView> swapChainImageViews;

        vk::RenderPass renderPass;
        vk::Image colorImage;
        vk::DeviceMemory colorImageMemory;
        vk::ImageView colorImageView;
        vk::Image depthImage;
        vk::DeviceMemory depthImageMemory;
        vk::ImageView depthImageView;
        std::vector<vk::Framebuffer> swapChainFramebuffers;
        std::vector<vk::ClearValue> clearValues;

        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR & capabilities);
        vk::Format findDepthFormat(zvlk::Device* device);

    };
}
#endif /* FRAME_H */

