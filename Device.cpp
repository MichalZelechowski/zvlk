/* 
 * File:   Device.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 11 czerwca 2020, 23:17
 */

#include "Device.h"

#include <iomanip>
#include <set>
#include <tuple>
#include <string.h>

template<typename T> void printProperty(std::ostream& os, const char* name, T value, const int& width) {
    std::cout << std::left << std::setw(width) << std::setfill(' ') << name << ": " << value << std::endl;
}

namespace zvlk {

    Device::Device(VkPhysicalDevice physicalDevice) {
        this->physicalDevice = physicalDevice;

        vkGetPhysicalDeviceProperties(this->physicalDevice, &this->deviceProperties);
        vkGetPhysicalDeviceFeatures(this->physicalDevice, &this->deviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memoryProperties);

        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
        this->availableExtensions.resize(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDevice, &queueFamilyCount, nullptr);
        this->queueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDevice, &queueFamilyCount, this->queueFamilies.data());

        this->graphicsDevice = VK_NULL_HANDLE;
        this->graphicsQueue = VK_NULL_HANDLE;
        this->presentQueue = VK_NULL_HANDLE;
        this->commandPool = VK_NULL_HANDLE;
    }

    const VkPhysicalDeviceProperties& Device::getProperties() {
        return this->deviceProperties;
    }

    const VkPhysicalDeviceFeatures& Device::getFeatures() {
        return this->deviceFeatures;
    }

    const VkPhysicalDeviceMemoryProperties& Device::getMemoryProperties() {
        return this->memoryProperties;
    }

    zvlk::Frame* Device::initializeForGraphics(VkSurfaceKHR surface, const std::vector<const char*> validationLayers, const std::vector<const char*> deviceExtensions) {
        zvlk::QueueFamilyIndices indices = this->findQueueFamilies(surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : indices.getUniqueQueueFamilies()) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        //TODO this is not always required, customize?
        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t> (queueCreateInfos.size());

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t> (deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (!validationLayers.empty()) {
            createInfo.enabledLayerCount = static_cast<uint32_t> (validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &graphicsDevice) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(this->graphicsDevice, indices.graphicsFamily, 0, &this->graphicsQueue);
        vkGetDeviceQueue(this->graphicsDevice, indices.presentFamily, 0, &this->presentQueue);

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = indices.graphicsFamily;

        if (vkCreateCommandPool(this->graphicsDevice, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }

        zvlk::Frame* result = new zvlk::Frame(this, surface);
        return result;
    }

    zvlk::SwapChainSupportDetails Device::querySwapChainSupport(VkSurfaceKHR surface) {
        zvlk::SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->physicalDevice, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(this->physicalDevice, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(this->physicalDevice, surface, &presentModeCount, details.presentModes.data());
        }
    }

    zvlk::QueueFamilyIndices Device::findQueueFamilies(VkSurfaceKHR surface) {
        zvlk::QueueFamilyIndices indices;
        indices.graphicsFamily = -1;

        int i = 0;
        for (const VkQueueFamilyProperties& queueFamily : this->queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(this->physicalDevice, i, surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }
        return indices;
    }

    bool Device::doesSupportExtensions(const std::vector<const char*> extensions) {
        std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

        for (const auto& extension : this->availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    bool Device::doesSupportGraphics(VkSurfaceKHR surface) {
        return this->findQueueFamilies(surface).isComplete();
    }

    Device::~Device() {
        if (this->graphicsDevice != VK_NULL_HANDLE) {
            if (this->commandPool != VK_NULL_HANDLE) {
                vkDestroyCommandPool(this->graphicsDevice, this->commandPool, nullptr);
            }
            vkDestroyDevice(this->graphicsDevice, nullptr);
        }
    }

    const VkFormatProperties Device::getFormatProperties(VkFormat format) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(this->physicalDevice, format, &props);
        return props;
    }

    VkSampleCountFlagBits Device::getMaxUsableSampleCount() {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(this->physicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) {
            return VK_SAMPLE_COUNT_64_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_32_BIT) {
            return VK_SAMPLE_COUNT_32_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_16_BIT) {
            return VK_SAMPLE_COUNT_16_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_8_BIT) {
            return VK_SAMPLE_COUNT_8_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_4_BIT) {
            return VK_SAMPLE_COUNT_4_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_2_BIT) {
            return VK_SAMPLE_COUNT_2_BIT;
        }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    void Device::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
        VkCommandBuffer commandBuffer = this->beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            bool hasStencilComponent = format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
            if (hasStencilComponent) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
                );

        this->endSingleTimeCommands(commandBuffer);
    }

    void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(this->graphicsDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(this->graphicsDevice, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = this->findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(this->graphicsDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(this->graphicsDevice, buffer, bufferMemory, 0);
    }

    void Device::createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
            VkSampleCountFlagBits numSamples,
            VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = numSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(this->graphicsDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(this->graphicsDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = this->findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(this->graphicsDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(this->graphicsDevice, image, imageMemory, 0);
    }

    VkImageView Device::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(this->graphicsDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    void Device::copyMemory(VkDeviceSize size, void* content, VkBuffer& stagingBuffer, VkDeviceMemory& stagingBufferMemory) {
        this->createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        this->copyMemory(size, content, stagingBufferMemory);
    }

    void Device::copyMemory(VkDeviceSize size, void* content, VkDeviceMemory& memory) {
        void* data;
        vkMapMemory(this->graphicsDevice, memory, 0, size, 0, &data);
        memcpy(data, content, static_cast<size_t> (size));
        vkUnmapMemory(this->graphicsDevice, memory);
    }

    void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = this->beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        this->endSingleTimeCommands(commandBuffer);
    }

    void Device::freeMemory(VkBuffer buffer, VkDeviceMemory memory) {
        vkDestroyBuffer(this->graphicsDevice, buffer, nullptr);
        vkFreeMemory(this->graphicsDevice, memory, nullptr);
    }

    void Device::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = this->beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
                );

        endSingleTimeCommands(commandBuffer);
    }

    uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        for (uint32_t i = 0; i < this->memoryProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (this->memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    VkCommandBuffer Device::beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = this->commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(this->graphicsDevice, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(this->graphicsQueue);

        vkFreeCommandBuffers(this->graphicsDevice, this->commandPool, 1, &commandBuffer);
    }

    void Device::freeCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers) {
        for (VkCommandBuffer commandBuffer : commandBuffers) {
            vkFreeCommandBuffers(this->graphicsDevice, this->commandPool, commandBuffers.size(), commandBuffers.data());
        }
    }

    void Device::allocateCommandBuffers(uint32_t frameNumbers, std::vector<VkCommandBuffer>& commandBuffers) {
        commandBuffers.resize(frameNumbers);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = this->commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

        if (vkAllocateCommandBuffers(this->graphicsDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void Device::submitGraphics(VkSubmitInfo* submitInfo, VkFence fence) {
        if (vkQueueSubmit(this->graphicsQueue, 1, submitInfo, fence) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
    }
    
    VkResult Device::present(VkPresentInfoKHR* presentInfo) {
        return vkQueuePresentKHR(this->presentQueue, presentInfo);
    }

    std::ostream& operator<<(std::ostream& os, const Device& device) {
        //TODO complete
        os << std::setfill('-') << std::setw(41) << "Device properties" << std::endl;
        printProperty(os, "apiVersion", device.deviceProperties.apiVersion, 20);
        printProperty(os, "driverVersion", device.deviceProperties.driverVersion, 20);
        printProperty(os, "vendorID", device.deviceProperties.vendorID, 20);
        printProperty(os, "deviceID", device.deviceProperties.deviceID, 20);
        printProperty(os, "deviceType", device.deviceProperties.deviceType, 20);
        printProperty(os, "pipelineCacheUUID", device.deviceProperties.pipelineCacheUUID, 20);

        os << std::setfill('-') << std::setw(41) << "Limits" << std::endl;
        printProperty(os, "maxImageDimension1D", device.deviceProperties.limits.maxImageDimension1D, 20);
        printProperty(os, "maxImageDimension2D", device.deviceProperties.limits.maxImageDimension2D, 20);
        printProperty(os, "maxImageDimension3D", device.deviceProperties.limits.maxImageDimension3D, 20);
        printProperty(os, "maxImageDimensionCube", device.deviceProperties.limits.maxImageDimensionCube, 20);
        printProperty(os, "maxImageArrayLayers", device.deviceProperties.limits.maxImageArrayLayers, 20);
        printProperty(os, "maxTexelBufferElements", device.deviceProperties.limits.maxTexelBufferElements, 20);
        printProperty(os, "maxUniformBufferRange", device.deviceProperties.limits.maxUniformBufferRange, 20);
        printProperty(os, "maxStorageBufferRange", device.deviceProperties.limits.maxStorageBufferRange, 20);
        printProperty(os, "maxPushConstantsSize", device.deviceProperties.limits.maxPushConstantsSize, 20);
        printProperty(os, "maxMemoryAllocationCount", device.deviceProperties.limits.maxMemoryAllocationCount, 20);
        printProperty(os, "maxSamplerAllocationCount", device.deviceProperties.limits.maxSamplerAllocationCount, 20);
        printProperty(os, "bufferImageGranularity", device.deviceProperties.limits.bufferImageGranularity, 20);
        printProperty(os, "sparseAddressSpaceSize", device.deviceProperties.limits.sparseAddressSpaceSize, 20);
        printProperty(os, "maxBoundDescriptorSets", device.deviceProperties.limits.maxBoundDescriptorSets, 20);
        printProperty(os, "maxPerStageDescriptorSamplers", device.deviceProperties.limits.maxPerStageDescriptorSamplers, 20);
        printProperty(os, "maxPerStageDescriptorUniformBuffers", device.deviceProperties.limits.maxPerStageDescriptorUniformBuffers, 20);
        printProperty(os, "maxPerStageDescriptorStorageBuffers", device.deviceProperties.limits.maxPerStageDescriptorStorageBuffers, 20);
        printProperty(os, "maxPerStageDescriptorSampledImages", device.deviceProperties.limits.maxPerStageDescriptorSampledImages, 20);
        printProperty(os, "maxPerStageDescriptorStorageImages", device.deviceProperties.limits.maxPerStageDescriptorStorageImages, 20);
        printProperty(os, "maxPerStageDescriptorInputAttachments", device.deviceProperties.limits.maxPerStageDescriptorInputAttachments, 20);
        printProperty(os, "maxPerStageResources", device.deviceProperties.limits.maxPerStageResources, 20);
        printProperty(os, "maxDescriptorSetSamplers", device.deviceProperties.limits.maxDescriptorSetSamplers, 20);
        printProperty(os, "maxDescriptorSetUniformBuffers", device.deviceProperties.limits.maxDescriptorSetUniformBuffers, 20);
        printProperty(os, "maxDescriptorSetUniformBuffersDynamic", device.deviceProperties.limits.maxDescriptorSetUniformBuffersDynamic, 20);
        printProperty(os, "maxDescriptorSetStorageBuffers", device.deviceProperties.limits.maxDescriptorSetStorageBuffers, 20);
        printProperty(os, "maxDescriptorSetStorageBuffersDynamic", device.deviceProperties.limits.maxDescriptorSetStorageBuffersDynamic, 20);
        printProperty(os, "maxDescriptorSetSampledImages", device.deviceProperties.limits.maxDescriptorSetSampledImages, 20);
        printProperty(os, "maxDescriptorSetStorageImages", device.deviceProperties.limits.maxDescriptorSetStorageImages, 20);
        printProperty(os, "maxDescriptorSetInputAttachments", device.deviceProperties.limits.maxDescriptorSetInputAttachments, 20);
        printProperty(os, "maxVertexInputAttributes", device.deviceProperties.limits.maxVertexInputAttributes, 20);
        printProperty(os, "maxVertexInputBindings", device.deviceProperties.limits.maxVertexInputBindings, 20);
        printProperty(os, "maxVertexInputAttributeOffset", device.deviceProperties.limits.maxVertexInputAttributeOffset, 20);
        printProperty(os, "maxVertexInputBindingStride", device.deviceProperties.limits.maxVertexInputBindingStride, 20);
        printProperty(os, "maxVertexOutputComponents", device.deviceProperties.limits.maxVertexOutputComponents, 20);
        printProperty(os, "maxTessellationGenerationLevel", device.deviceProperties.limits.maxTessellationGenerationLevel, 20);
        printProperty(os, "maxTessellationPatchSize", device.deviceProperties.limits.maxTessellationPatchSize, 20);
        printProperty(os, "maxTessellationControlPerVertexInputComponents", device.deviceProperties.limits.maxTessellationControlPerVertexInputComponents, 20);
        printProperty(os, "maxTessellationControlPerVertexOutputComponents", device.deviceProperties.limits.maxTessellationControlPerVertexOutputComponents, 20);
        printProperty(os, "maxTessellationControlPerPatchOutputComponents", device.deviceProperties.limits.maxTessellationControlPerPatchOutputComponents, 20);
        printProperty(os, "maxTessellationControlTotalOutputComponents", device.deviceProperties.limits.maxTessellationControlTotalOutputComponents, 20);
        printProperty(os, "maxTessellationEvaluationInputComponents", device.deviceProperties.limits.maxTessellationEvaluationInputComponents, 20);
        printProperty(os, "maxTessellationEvaluationOutputComponents", device.deviceProperties.limits.maxTessellationEvaluationOutputComponents, 20);
        printProperty(os, "maxGeometryShaderInvocations", device.deviceProperties.limits.maxGeometryShaderInvocations, 20);
        printProperty(os, "maxGeometryInputComponents", device.deviceProperties.limits.maxGeometryInputComponents, 20);
        printProperty(os, "maxGeometryOutputComponents", device.deviceProperties.limits.maxGeometryOutputComponents, 20);
        printProperty(os, "maxGeometryOutputVertices", device.deviceProperties.limits.maxGeometryOutputVertices, 20);
        printProperty(os, "maxGeometryTotalOutputComponents", device.deviceProperties.limits.maxGeometryTotalOutputComponents, 20);
        printProperty(os, "maxFragmentInputComponents", device.deviceProperties.limits.maxFragmentInputComponents, 20);
        printProperty(os, "maxFragmentOutputAttachments", device.deviceProperties.limits.maxFragmentOutputAttachments, 20);
        printProperty(os, "maxFragmentDualSrcAttachments", device.deviceProperties.limits.maxFragmentDualSrcAttachments, 20);
        printProperty(os, "maxFragmentCombinedOutputResources", device.deviceProperties.limits.maxFragmentCombinedOutputResources, 20);
        printProperty(os, "maxComputeSharedMemorySize", device.deviceProperties.limits.maxComputeSharedMemorySize, 20);
        printProperty(os, "maxComputeWorkGroupCount[3]", device.deviceProperties.limits.maxComputeWorkGroupCount[3], 20);
        printProperty(os, "maxComputeWorkGroupInvocations", device.deviceProperties.limits.maxComputeWorkGroupInvocations, 20);
        printProperty(os, "maxComputeWorkGroupSize[3]", device.deviceProperties.limits.maxComputeWorkGroupSize[3], 20);
        printProperty(os, "subPixelPrecisionBits", device.deviceProperties.limits.subPixelPrecisionBits, 20);
        printProperty(os, "subTexelPrecisionBits", device.deviceProperties.limits.subTexelPrecisionBits, 20);
        printProperty(os, "mipmapPrecisionBits", device.deviceProperties.limits.mipmapPrecisionBits, 20);
        printProperty(os, "maxDrawIndexedIndexValue", device.deviceProperties.limits.maxDrawIndexedIndexValue, 20);
        printProperty(os, "maxDrawIndirectCount", device.deviceProperties.limits.maxDrawIndirectCount, 20);
        printProperty(os, "maxSamplerLodBias", device.deviceProperties.limits.maxSamplerLodBias, 20);
        printProperty(os, "maxSamplerAnisotropy", device.deviceProperties.limits.maxSamplerAnisotropy, 20);
        printProperty(os, "maxViewports", device.deviceProperties.limits.maxViewports, 20);
        printProperty(os, "maxViewportDimensions[2]", device.deviceProperties.limits.maxViewportDimensions[2], 20);
        printProperty(os, "viewportBoundsRange[2]", device.deviceProperties.limits.viewportBoundsRange[2], 20);
        printProperty(os, "viewportSubPixelBits", device.deviceProperties.limits.viewportSubPixelBits, 20);
        printProperty(os, "minMemoryMapAlignment", device.deviceProperties.limits.minMemoryMapAlignment, 20);
        printProperty(os, "minTexelBufferOffsetAlignment", device.deviceProperties.limits.minTexelBufferOffsetAlignment, 20);
        printProperty(os, "minUniformBufferOffsetAlignment", device.deviceProperties.limits.minUniformBufferOffsetAlignment, 20);
        printProperty(os, "minStorageBufferOffsetAlignment", device.deviceProperties.limits.minStorageBufferOffsetAlignment, 20);
        printProperty(os, "minTexelOffset", device.deviceProperties.limits.minTexelOffset, 20);
        printProperty(os, "maxTexelOffset", device.deviceProperties.limits.maxTexelOffset, 20);
        printProperty(os, "minTexelGatherOffset", device.deviceProperties.limits.minTexelGatherOffset, 20);
        printProperty(os, "maxTexelGatherOffset", device.deviceProperties.limits.maxTexelGatherOffset, 20);
        printProperty(os, "minInterpolationOffset", device.deviceProperties.limits.minInterpolationOffset, 20);
        printProperty(os, "maxInterpolationOffset", device.deviceProperties.limits.maxInterpolationOffset, 20);
        printProperty(os, "subPixelInterpolationOffsetBits", device.deviceProperties.limits.subPixelInterpolationOffsetBits, 20);
        printProperty(os, "maxFramebufferWidth", device.deviceProperties.limits.maxFramebufferWidth, 20);
        printProperty(os, "maxFramebufferHeight", device.deviceProperties.limits.maxFramebufferHeight, 20);
        printProperty(os, "maxFramebufferLayers", device.deviceProperties.limits.maxFramebufferLayers, 20);
        printProperty(os, "framebufferColorSampleCounts", device.deviceProperties.limits.framebufferColorSampleCounts, 20);
        printProperty(os, "framebufferDepthSampleCounts", device.deviceProperties.limits.framebufferDepthSampleCounts, 20);
        printProperty(os, "framebufferStencilSampleCounts", device.deviceProperties.limits.framebufferStencilSampleCounts, 20);
        printProperty(os, "framebufferNoAttachmentsSampleCounts", device.deviceProperties.limits.framebufferNoAttachmentsSampleCounts, 20);
        printProperty(os, "maxColorAttachments", device.deviceProperties.limits.maxColorAttachments, 20);
        printProperty(os, "sampledImageColorSampleCounts", device.deviceProperties.limits.sampledImageColorSampleCounts, 20);
        printProperty(os, "sampledImageIntegerSampleCounts", device.deviceProperties.limits.sampledImageIntegerSampleCounts, 20);
        printProperty(os, "sampledImageDepthSampleCounts", device.deviceProperties.limits.sampledImageDepthSampleCounts, 20);
        printProperty(os, "sampledImageStencilSampleCounts", device.deviceProperties.limits.sampledImageStencilSampleCounts, 20);
        printProperty(os, "storageImageSampleCounts", device.deviceProperties.limits.storageImageSampleCounts, 20);
        printProperty(os, "maxSampleMaskWords", device.deviceProperties.limits.maxSampleMaskWords, 20);
        printProperty(os, "timestampComputeAndGraphics", device.deviceProperties.limits.timestampComputeAndGraphics, 20);
        printProperty(os, "timestampPeriod", device.deviceProperties.limits.timestampPeriod, 20);
        printProperty(os, "maxClipDistances", device.deviceProperties.limits.maxClipDistances, 20);
        printProperty(os, "maxCullDistances", device.deviceProperties.limits.maxCullDistances, 20);
        printProperty(os, "maxCombinedClipAndCullDistances", device.deviceProperties.limits.maxCombinedClipAndCullDistances, 20);
        printProperty(os, "discreteQueuePriorities", device.deviceProperties.limits.discreteQueuePriorities, 20);
        printProperty(os, "pointSizeRange[2]", device.deviceProperties.limits.pointSizeRange[2], 20);
        printProperty(os, "lineWidthRange[2]", device.deviceProperties.limits.lineWidthRange[2], 20);
        printProperty(os, "pointSizeGranularity", device.deviceProperties.limits.pointSizeGranularity, 20);
        printProperty(os, "lineWidthGranularity", device.deviceProperties.limits.lineWidthGranularity, 20);
        printProperty(os, "strictLines", device.deviceProperties.limits.strictLines, 20);
        printProperty(os, "standardSampleLocations", device.deviceProperties.limits.standardSampleLocations, 20);
        printProperty(os, "optimalBufferCopyOffsetAlignment", device.deviceProperties.limits.optimalBufferCopyOffsetAlignment, 20);
        printProperty(os, "optimalBufferCopyRowPitchAlignment", device.deviceProperties.limits.optimalBufferCopyRowPitchAlignment, 20);
        printProperty(os, "nonCoherentAtomSize", device.deviceProperties.limits.nonCoherentAtomSize, 20);

        os << std::setfill('-') << std::setw(41) << "Sparse properties" << std::endl;

        printProperty(os, "residencyStandard2DBlockShape", device.deviceProperties.sparseProperties.residencyStandard2DBlockShape, 20);
        printProperty(os, "residencyStandard2DMultisampleBlockShape", device.deviceProperties.sparseProperties.residencyStandard2DMultisampleBlockShape, 20);
        printProperty(os, "residencyStandard3DBlockShape", device.deviceProperties.sparseProperties.residencyStandard3DBlockShape, 20);
        printProperty(os, "residencyAlignedMipSize", device.deviceProperties.sparseProperties.residencyAlignedMipSize, 20);
        printProperty(os, "residencyNonResidentStrict", device.deviceProperties.sparseProperties.residencyNonResidentStrict, 20);

        return os;
    }


}