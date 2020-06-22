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
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    } SwapChainSupportDetails;

    class Device {
    public:
        Device() = delete;
        Device(const Device& orig) = delete;

        Device(VkPhysicalDevice physicalDevice);
        virtual ~Device();

        VkPhysicalDevice getDevice() {
            return this->physicalDevice;
        }

        VkDevice getGraphicsDevice() {
            return this->graphicsDevice;
        }

        const VkPhysicalDeviceProperties& getProperties();
        const VkPhysicalDeviceFeatures& getFeatures();
        const VkPhysicalDeviceMemoryProperties& getMemoryProperties();
        const VkFormatProperties getFormatProperties(VkFormat format);
        VkSampleCountFlagBits getMaxUsableSampleCount();

        zvlk::Frame* initializeForGraphics(VkSurfaceKHR surface, const std::vector<const char*>, const std::vector<const char*> deviceExtensions);

        bool doesSupportExtensions(const std::vector<const char*> extensions);
        bool doesSupportGraphics(VkSurfaceKHR surface);

        zvlk::SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface);
        zvlk::QueueFamilyIndices findQueueFamilies(VkSurfaceKHR surface);
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
        void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                VkSampleCountFlagBits numSamples,
                VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void copyMemory(VkDeviceSize size, void* content, VkBuffer& buffer, VkDeviceMemory& memory);
        void copyMemory(VkDeviceSize size, void* content, VkDeviceMemory& memory);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void freeMemory(VkBuffer buffer, VkDeviceMemory memory);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void freeCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);

        void allocateCommandBuffers(uint32_t frameNumbers, std::vector<VkCommandBuffer>& commandBuffers);

        void submitGraphics(VkSubmitInfo* submitInfo, VkFence fence);
        VkResult present(VkPresentInfoKHR* presentInfo);
        
        friend std::ostream& operator<<(std::ostream& os, const Device& device);
    private:
        VkPhysicalDevice physicalDevice;
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        VkPhysicalDeviceMemoryProperties memoryProperties;
        std::vector<VkExtensionProperties> availableExtensions;
        std::vector<VkQueueFamilyProperties> queueFamilies;
        VkDevice graphicsDevice;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkCommandPool commandPool;

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };
}
#endif /* DEVICE_H */
