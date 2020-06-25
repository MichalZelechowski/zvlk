/* 
 * File:   Device.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 11 czerwca 2020, 23:17
 */

#ifndef DEVICE_H
#define DEVICE_H

#include <vulkan/vulkan.hpp>
#include <vector>
#include <iostream>
#include <set>

#include "Frame.h"

namespace zvlk {

    class Frame;

    typedef struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t presentFamily;

        bool isComplete() {
            return graphicsFamily != -1U && presentFamily != -1U;
        }

        std::set<uint32_t> getUniqueQueueFamilies() {
            return {graphicsFamily, presentFamily};
        }

    } QueueFamilyIndices;

    typedef struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    } SwapChainSupportDetails;

    class Device {
    public:
        Device() = delete;
        Device(const Device& orig) = delete;

        Device(vk::PhysicalDevice physicalDevice);
        virtual ~Device();

        vk::PhysicalDevice getDevice() {
            return this->physicalDevice;
        }

        vk::Device getGraphicsDevice() {
            return this->graphicsDevice;
        }

        const vk::PhysicalDeviceProperties& getProperties();
        const vk::PhysicalDeviceFeatures& getFeatures();
        const vk::PhysicalDeviceMemoryProperties& getMemoryProperties();
        const vk::FormatProperties getFormatProperties(vk::Format format);
        vk::SampleCountFlagBits getMaxUsableSampleCount();

        zvlk::Frame* initializeForGraphics(vk::SurfaceKHR surface, const std::vector<const char*>, const std::vector<const char*> deviceExtensions);

        bool doesSupportExtensions(const std::vector<const char*> extensions);
        bool doesSupportGraphics(vk::SurfaceKHR surface);

        zvlk::SwapChainSupportDetails querySwapChainSupport(vk::SurfaceKHR surface);
        zvlk::QueueFamilyIndices findQueueFamilies(vk::SurfaceKHR surface);
        void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);

        vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);
        void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                vk::SampleCountFlagBits numSamples,
                vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory);
        void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
        void copyMemory(vk::DeviceSize size, void* content, vk::Buffer& buffer, vk::DeviceMemory& memory);
        void copyMemory(vk::DeviceSize size, void* content, vk::DeviceMemory& memory);
        void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
        void freeMemory(vk::Buffer buffer, vk::DeviceMemory memory);
        void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
        void freeCommandBuffers(std::vector<vk::CommandBuffer>& commandBuffers);
        vk::CommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

        void allocateCommandBuffers(uint32_t frameNumbers, std::vector<vk::CommandBuffer>& commandBuffers);

        void submitGraphics(vk::SubmitInfo* submitInfo, vk::Fence fence);
        vk::Result present(vk::PresentInfoKHR* presentInfo);
        
        friend std::ostream& operator<<(std::ostream& os, const Device& device);
    private:
        vk::PhysicalDevice physicalDevice;
        vk::PhysicalDeviceProperties deviceProperties;
        vk::PhysicalDeviceFeatures deviceFeatures;
        vk::PhysicalDeviceMemoryProperties memoryProperties;
        std::vector<vk::ExtensionProperties> availableExtensions;
        std::vector<vk::QueueFamilyProperties> queueFamilies;
        vk::Device graphicsDevice;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        vk::CommandPool commandPool;

        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    };
}
#endif /* DEVICE_H */
