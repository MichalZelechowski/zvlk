/* 
 * File:   Frame.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 16 czerwca 2020, 22:57
 */

#include "Frame.h"

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <array>
#include <tuple>

namespace zvlk {

    Frame::~Frame() {
        for (auto imageView : this->swapChainImageViews) {
            this->graphicsDevice.destroy(imageView);
        }

        this->graphicsDevice.destroy(this->renderPass);
        this->graphicsDevice.destroy(this->swapChain);

        this->graphicsDevice.destroy(this->colorImageView);
        this->graphicsDevice.destroy(this->colorImage);
        this->graphicsDevice.free(this->colorImageMemory);

        this->graphicsDevice.destroy(this->depthImageView);
        this->graphicsDevice.destroy(this->depthImage);
        this->graphicsDevice.free(this->depthImageMemory);

        for (auto framebuffer : this->swapChainFramebuffers) {
            this->graphicsDevice.destroy(framebuffer);
        }
    }

    uint32_t Frame::getImagesNumber() {
        return this->swapChainImages.size();
    }

    uint32_t Frame::getWidth() {
        return this->swapChainExtent.width;
    }

    uint32_t Frame::getHeight() {
        return this->swapChainExtent.height;
    }

    Frame::Frame(zvlk::Device* device, vk::SurfaceKHR surface) {
        this->graphicsDevice = device->getGraphicsDevice();

        zvlk::SwapChainSupportDetails swapChainSupport = device->querySwapChainSupport(surface);

        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo({}, surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
                extent, 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, 0, nullptr,
                swapChainSupport.capabilities.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque,
                presentMode, VK_TRUE);

        std::set<uint32_t> uniqueQueueFamiliesSet = device->findQueueFamilies(surface).getUniqueQueueFamilies();
        std::vector<uint32_t> uniqueQueueFamilies(uniqueQueueFamiliesSet.begin(), uniqueQueueFamiliesSet.end());
        if (uniqueQueueFamilies.size() == 2) { //if ownership has to be transferred between  queues (for multiple queues)
            createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
                    .setQueueFamilyIndexCount(2)
                    .setPQueueFamilyIndices(uniqueQueueFamilies.data());
        }

        this->swapChain = this->graphicsDevice.createSwapchainKHR(createInfo);
        this->swapChainImages = this->graphicsDevice.getSwapchainImagesKHR(this->swapChain);

        this->swapChainExtent = extent;
        this->swapChainImageFormat = surfaceFormat.format;

        this->swapChainImageViews.resize(this->swapChainImages.size());
        for (size_t i = 0; i < this->swapChainImages.size(); i++) {
            swapChainImageViews[i] = device->createImageView(swapChainImages[i], swapChainImageFormat, vk::ImageAspectFlagBits::eColor, 1);
        }

        vk::SampleCountFlagBits msaaSamples = device->getMaxUsableSampleCount();
        vk::Format depthFormat = this->findDepthFormat(device);

        vk::AttachmentDescription colorAttachment({}, swapChainImageFormat, msaaSamples,
                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
        vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

        vk::AttachmentDescription depthAttachment({}, depthFormat, msaaSamples,
                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
                vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
        vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::AttachmentDescription colorAttachmentResolve({}, swapChainImageFormat,
                vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eStore,
                vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
        vk::AttachmentReference colorAttachmentResolveRef(2, vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, 0, nullptr,
                1, &colorAttachmentRef, &colorAttachmentResolveRef, &depthAttachmentRef);

        vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL, 0,
                vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
        {}, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

        std::array<vk::AttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
        vk::RenderPassCreateInfo renderPassInfo({},
        static_cast<uint32_t> (attachments.size()), attachments.data(),
                1, &subpass,
                1, &dependency);

        this->renderPass = this->graphicsDevice.createRenderPass(renderPassInfo);

        device->createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples,
                swapChainImageFormat, vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
                vk::MemoryPropertyFlagBits::eDeviceLocal, colorImage, colorImageMemory);
        this->colorImageView = device->createImageView(colorImage, swapChainImageFormat, vk::ImageAspectFlagBits::eColor, 1);

        device->createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat,
                vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
                vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
        this->depthImageView = device->createImageView(this->depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);

        device->transitionImageLayout(this->depthImage, depthFormat, vk::ImageLayout::eUndefined,
                vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);

        swapChainFramebuffers.resize(this->swapChainImageViews.size());

        for (size_t i = 0; i < this->swapChainImageViews.size(); i++) {
            std::array<vk::ImageView, 3> attachments = {
                colorImageView,
                depthImageView,
                swapChainImageViews[i]
            };

            vk::FramebufferCreateInfo framebufferInfo({}, this->renderPass,
                    static_cast<uint32_t> (attachments.size()), attachments.data(),
                    swapChainExtent.width, swapChainExtent.height,
                    1);
            swapChainFramebuffers[i] = this->graphicsDevice.createFramebuffer(framebufferInfo);
        }

        this->clearValues = {vk::ClearValue(vk::ClearColorValue(std::array<float,4>({1.0f, 0.0f, 0.0f, 1.0f}))),
            vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0)),
            vk::ClearValue(vk::ClearColorValue(std::array<float,4>({1.0f, 0.0f, 0.0f, 1.0f})))};
    }

    vk::SurfaceFormatKHR Frame::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
        for (const vk::SurfaceFormatKHR& availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
                    availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    vk::Format Frame::findDepthFormat(zvlk::Device* device) {
        const std::vector<vk::Format> candidates = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};
        const vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
        const vk::FormatFeatureFlagBits features = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

        for (vk::Format format : candidates) {
            vk::FormatProperties props = device->getFormatProperties(format);

            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    vk::PresentModeKHR Frame::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
        for (const vk::PresentModeKHR& availablePresentMode : availablePresentModes) {
            // this can be customized if lower cpu load is required
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D Frame::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR & capabilities) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            // TODO this is clearly related to window, has to hook it up somehow
            //std::tuple<int, int> t = this->window->getSize()
            vk::Extent2D actualExtent(800, 600);

            actualExtent.setWidth(std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width)));
            actualExtent.setHeight(std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height)));

            return actualExtent;
        }
    }

    vk::RenderPass Frame::getRenderPass() {
        return this->renderPass;
    }

    vk::RenderPassBeginInfo Frame::getRenderPassBeginInfo(uint32_t index) const {
        vk::RenderPassBeginInfo renderPassInfo(this->renderPass, this->swapChainFramebuffers[index],
                vk::Rect2D(vk::Offset2D(0, 0), this->swapChainExtent),
                static_cast<uint32_t> (clearValues.size()),
                clearValues.data());
        return renderPassInfo;
    }
}

