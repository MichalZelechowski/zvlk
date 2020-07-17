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

    Device::Device(vk::PhysicalDevice physicalDevice) {
        this->physicalDevice = physicalDevice;

        this->deviceProperties = this->physicalDevice.getProperties();
        this->deviceFeatures = this->physicalDevice.getFeatures();
        this->memoryProperties = this->physicalDevice.getMemoryProperties();

        this->availableExtensions = this->physicalDevice.enumerateDeviceExtensionProperties();
        this->queueFamilies = this->physicalDevice.getQueueFamilyProperties();
    }

    const vk::PhysicalDeviceProperties& Device::getProperties() {
        return this->deviceProperties;
    }

    const vk::PhysicalDeviceFeatures& Device::getFeatures() {
        return this->deviceFeatures;
    }

    const vk::PhysicalDeviceMemoryProperties& Device::getMemoryProperties() {
        return this->memoryProperties;
    }

    std::shared_ptr<zvlk::Frame> Device::initializeForGraphics(vk::SurfaceKHR surface, const std::vector<const char*> validationLayers, const std::vector<const char*> deviceExtensions) {
        zvlk::QueueFamilyIndices indices = this->findQueueFamilies(surface);

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : indices.getUniqueQueueFamilies()) {
            queueCreateInfos.push_back({
                {}, queueFamily, 1, &queuePriority
            });
        }

        vk::PhysicalDeviceFeatures deviceFeatures;
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;

        vk::DeviceCreateInfo createInfo({}, static_cast<uint32_t> (queueCreateInfos.size()),
                queueCreateInfos.data(),
                static_cast<uint32_t> (validationLayers.size()),
                validationLayers.data(),
                static_cast<uint32_t> (deviceExtensions.size()),
                deviceExtensions.data(),
                &deviceFeatures);

        this->graphicsDevice = this->physicalDevice.createDevice(createInfo);

        this->graphicsQueue = this->graphicsDevice.getQueue(indices.graphicsFamily, 0);
        this->presentQueue = this->graphicsDevice.getQueue(indices.presentFamily, 0);

        this->commandPool = this->graphicsDevice.createCommandPool({
            {}, indices.graphicsFamily
        });

        std::shared_ptr<zvlk::Frame> result(new zvlk::Frame(this, (VkSurfaceKHR) surface));
        return result;
    }

    zvlk::SwapChainSupportDetails Device::querySwapChainSupport(vk::SurfaceKHR surface) {
        return {
            this->physicalDevice.getSurfaceCapabilitiesKHR(surface),
            this->physicalDevice.getSurfaceFormatsKHR(surface),
            this->physicalDevice.getSurfacePresentModesKHR(surface)};
    }

    zvlk::QueueFamilyIndices Device::findQueueFamilies(vk::SurfaceKHR surface) {
        zvlk::QueueFamilyIndices indices;
        indices.graphicsFamily = -1;

        int i = 0;
        for (const vk::QueueFamilyProperties& queueFamily : this->queueFamilies) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphicsFamily = i;
            }

            if (this->physicalDevice.getSurfaceSupportKHR(i, surface)) {
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

    bool Device::doesSupportGraphics(vk::SurfaceKHR surface) {
        return this->findQueueFamilies(surface).isComplete();
    }

    Device::~Device() {
        if (this->graphicsDevice) {
            if (this->commandPool) {
                this->graphicsDevice.destroy(this->commandPool);
            }
            this->graphicsDevice.destroy();
        }
    }

    const vk::FormatProperties Device::getFormatProperties(vk::Format format) {
        return this->physicalDevice.getFormatProperties(format);
    }

    vk::SampleCountFlagBits Device::getMaxUsableSampleCount() {
        vk::SampleCountFlags counts = this->deviceProperties.limits.framebufferColorSampleCounts &
                this->deviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & vk::SampleCountFlagBits::e64) {
            return vk::SampleCountFlagBits::e64;
        }
        if (counts & vk::SampleCountFlagBits::e32) {
            return vk::SampleCountFlagBits::e32;
        }
        if (counts & vk::SampleCountFlagBits::e16) {
            return vk::SampleCountFlagBits::e16;
        }
        if (counts & vk::SampleCountFlagBits::e8) {
            return vk::SampleCountFlagBits::e8;
        }
        if (counts & vk::SampleCountFlagBits::e4) {
            return vk::SampleCountFlagBits::e4;
        }
        if (counts & vk::SampleCountFlagBits::e2) {
            return vk::SampleCountFlagBits::e2;
        }

        return vk::SampleCountFlagBits::e1;
    }

    void Device::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels) {
        vk::CommandBuffer commandBuffer = this->beginSingleTimeCommands();

        vk::ImageMemoryBarrier barrier({},
        {
        }, oldLayout, newLayout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                image,{vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1});

        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destinationStage;

        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
        } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

            sourceStage = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
        } else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
            barrier.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
            bool hasStencilComponent = format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
            if (hasStencilComponent) {
                barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
            }
        } else {
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        }

        commandBuffer.pipelineBarrier(sourceStage, destinationStage,{}, 0, nullptr, 0, nullptr, 1, &barrier);

        this->endSingleTimeCommands(commandBuffer);
    }

    void Device::createVertexBuffer(vk::DeviceSize size, vk::Buffer& buffer) {
        vk::BufferCreateInfo bufferInfo({}, size,
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                vk::SharingMode::eExclusive);
        buffer = this->graphicsDevice.createBuffer(bufferInfo);
    }

    void Device::createIndexBuffer(vk::DeviceSize size, vk::Buffer& buffer) {
        vk::BufferCreateInfo bufferInfo({}, size,
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                vk::SharingMode::eExclusive);
        buffer = this->graphicsDevice.createBuffer(bufferInfo);
    }

    vk::MemoryAllocateInfo Device::getMemoryAllocateInfo(vk::Buffer buffer, vk::MemoryPropertyFlags properties) {
        vk::MemoryRequirements memRequirements = this->graphicsDevice.getBufferMemoryRequirements(buffer);
        vk::MemoryAllocateInfo allocInfo(memRequirements.size, this->findMemoryType(memRequirements.memoryTypeBits, properties));
        return allocInfo;
    }

    void Device::createDeviceMemory(vk::MemoryAllocateInfo info, vk::DeviceMemory& memory) {
        memory = this->graphicsDevice.allocateMemory(info);
    }

    void Device::bindBuffer(vk::Buffer& buffer, vk::DeviceMemory& memory, vk::DeviceSize offset) {
        this->graphicsDevice.bindBufferMemory(buffer, memory, offset);
    }

    void Device::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory) {
        vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
        buffer = this->graphicsDevice.createBuffer(bufferInfo);
        this->createDeviceMemory(this->getMemoryAllocateInfo(buffer, properties), bufferMemory);
        this->bindBuffer(buffer, bufferMemory, 0);
    }

    void Device::createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
            vk::SampleCountFlagBits numSamples,
            vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
            vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory) {
        vk::ImageCreateInfo imageInfo({}, vk::ImageType::e2D, format, vk::Extent3D(width, height, 1), mipLevels, 1, numSamples, tiling, usage, vk::SharingMode::eExclusive);
        image = this->graphicsDevice.createImage(imageInfo);

        vk::MemoryRequirements memRequirements = this->graphicsDevice.getImageMemoryRequirements(image);
        vk::MemoryAllocateInfo allocInfo(memRequirements.size, this->findMemoryType(memRequirements.memoryTypeBits, properties));
        imageMemory = this->graphicsDevice.allocateMemory(allocInfo);
        this->graphicsDevice.bindImageMemory(image, imageMemory, 0);
    }

    vk::ImageView Device::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels) {
        vk::ImageViewCreateInfo viewInfo({}, image, vk::ImageViewType::e2D, format, vk::ComponentMapping(),
                vk::ImageSubresourceRange(aspectFlags, 0, mipLevels, 0, 1));

        return this->graphicsDevice.createImageView(viewInfo);
    }

    void Device::createStagingBuffer(vk::DeviceSize size, vk::Buffer& stagingBuffer, vk::DeviceMemory& stagingBufferMemory) {
        this->createBuffer(size,
                vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                stagingBuffer, stagingBufferMemory);
    }

    void Device::copyMemory(vk::DeviceSize size, void* content, vk::Buffer& stagingBuffer, vk::DeviceMemory& stagingBufferMemory) {
        this->createStagingBuffer(size, stagingBuffer, stagingBufferMemory);
        this->copyMemory(size, content, stagingBufferMemory);
    }

    void Device::copyMemory(vk::DeviceSize size, void* content, vk::DeviceMemory& memory) {
        void* data = this->graphicsDevice.mapMemory(memory, 0, size);
        memcpy(data, content, static_cast<size_t> (size));
        this->graphicsDevice.unmapMemory(memory);
    }

    void Device::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size, vk::DeviceSize dstOffset) {
        vk::CommandBuffer commandBuffer = this->beginSingleTimeCommands();

        vk::BufferCopy copyRegion(0, dstOffset, size);
        commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

        this->endSingleTimeCommands(commandBuffer);
    }
    
    void Device::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
        this->copyBuffer(srcBuffer, dstBuffer, size, 0);
    }

    void Device::freeMemory(vk::Buffer buffer, vk::DeviceMemory memory) {
        this->graphicsDevice.destroy(buffer);
        this->graphicsDevice.freeMemory(memory);
    }

    void Device::freeMemory(std::vector<vk::Buffer> buffers, vk::DeviceMemory memory) {
        for (vk::Buffer& buffer : buffers) {
            this->graphicsDevice.destroy(buffer);
        }
        this->graphicsDevice.freeMemory(memory);
    }

    void Device::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
        vk::CommandBuffer commandBuffer = this->beginSingleTimeCommands();

        vk::BufferImageCopy region(0, 0, 0,
                vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
                vk::Offset3D(0, 0, 0),
                vk::Extent3D(width, height, 1));
        commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

        endSingleTimeCommands(commandBuffer);
    }

    uint32_t Device::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
        for (uint32_t i = 0; i < this->memoryProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (this->memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    vk::CommandBuffer Device::beginSingleTimeCommands() {
        vk::CommandBufferAllocateInfo allocInfo(this->commandPool, vk::CommandBufferLevel::ePrimary, 1);
        vk::CommandBuffer commandBuffer = this->graphicsDevice.allocateCommandBuffers(allocInfo)[0];

        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        commandBuffer.begin(beginInfo);
        return commandBuffer;
    }

    void Device::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
        commandBuffer.end();

        vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &commandBuffer, 0, nullptr);
        this->graphicsQueue.submit(1, &submitInfo, vk::Fence());
        this->graphicsQueue.waitIdle();

        this->graphicsDevice.free(this->commandPool, 1, &commandBuffer);
    }

    void Device::freeCommandBuffers(std::vector<vk::CommandBuffer>& commandBuffers) {
        this->graphicsDevice.free(this->commandPool, commandBuffers);
    }

    void Device::allocateCommandBuffers(uint32_t frameNumbers, std::vector<vk::CommandBuffer>& commandBuffers) {
        commandBuffers.resize(frameNumbers);

        vk::CommandBufferAllocateInfo allocInfo(this->commandPool, vk::CommandBufferLevel::ePrimary, (uint32_t) commandBuffers.size());
        this->graphicsDevice.allocateCommandBuffers(&allocInfo, commandBuffers.data());
    }

    void Device::submitGraphics(vk::SubmitInfo* submitInfo, vk::Fence fence) {
        this->graphicsQueue.submit(1, submitInfo, fence);
    }

    vk::Result Device::present(vk::PresentInfoKHR* presentInfo) {
        return this->presentQueue.presentKHR(presentInfo);
    }

    std::ostream& operator<<(std::ostream& os, const Device& device) {
        /*//TODO complete
        os << std::setfill('-') << std::setw(41) << "Device properties" << std::endl;
        printProperty(os, "apiVersion", device.deviceProperties.apiVersion, 20);
        printProperty(os, "driverVersion", device.deviceProperties.driverVersion, 20);
        printProperty(os, "vendorID", device.deviceProperties.vendorID, 20);
        printProperty(os, "deviceID", device.deviceProperties.deviceID, 20);
        //printProperty(os, "deviceType", device.deviceProperties.deviceType, 20);
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
        printProperty(os, "maxComputeWorkGroupCount[3]", device.deviceProperties.limits.maxComputeWorkGroupCount, 20);
        printProperty(os, "maxComputeWorkGroupInvocations", device.deviceProperties.limits.maxComputeWorkGroupInvocations, 20);
        printProperty(os, "maxComputeWorkGroupSize[3]", device.deviceProperties.limits.maxComputeWorkGroupSize, 20);
        printProperty(os, "subPixelPrecisionBits", device.deviceProperties.limits.subPixelPrecisionBits, 20);
        printProperty(os, "subTexelPrecisionBits", device.deviceProperties.limits.subTexelPrecisionBits, 20);
        printProperty(os, "mipmapPrecisionBits", device.deviceProperties.limits.mipmapPrecisionBits, 20);
        printProperty(os, "maxDrawIndexedIndexValue", device.deviceProperties.limits.maxDrawIndexedIndexValue, 20);
        printProperty(os, "maxDrawIndirectCount", device.deviceProperties.limits.maxDrawIndirectCount, 20);
        printProperty(os, "maxSamplerLodBias", device.deviceProperties.limits.maxSamplerLodBias, 20);
        printProperty(os, "maxSamplerAnisotropy", device.deviceProperties.limits.maxSamplerAnisotropy, 20);
        printProperty(os, "maxViewports", device.deviceProperties.limits.maxViewports, 20);
        printProperty(os, "maxViewportDimensions[2]", device.deviceProperties.limits.maxViewportDimensions, 20);
        printProperty(os, "viewportBoundsRange[2]", device.deviceProperties.limits.viewportBoundsRange, 20);
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
        printProperty(os, "pointSizeRange[2]", device.deviceProperties.limits.pointSizeRange, 20);
        printProperty(os, "lineWidthRange[2]", device.deviceProperties.limits.lineWidthRange, 20);
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
         */
        return os;
    }


}