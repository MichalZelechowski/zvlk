/* 
 * File:   Texture.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 17 czerwca 2020, 13:02
 */

#include "Texture.h"
#include "Device.h"
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace zvlk {

    Texture::Texture(Device* device, std::string texturePath) {
        this->device = device->getGraphicsDevice();

        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(texturePath.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        vk::DeviceSize imageSize = texWidth * texHeight * 4;
        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;
        device->copyMemory(imageSize, pixels, stagingBuffer, stagingBufferMemory);

        stbi_image_free(pixels);

        this->mipLevels = static_cast<uint32_t> (std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        device->createImage(texWidth, texHeight, this->mipLevels,
                vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eDeviceLocal, this->image, this->imageMemory);

        device->transitionImageLayout(this->image, vk::Format::eR8G8B8A8Srgb,
                vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, this->mipLevels);
        device->copyBufferToImage(stagingBuffer, this->image, static_cast<uint32_t> (texWidth), static_cast<uint32_t> (texHeight));

        device->freeMemory(stagingBuffer, stagingBufferMemory);

        if (!(device->getFormatProperties(vk::Format::eR8G8B8A8Srgb).optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        vk::CommandBuffer commandBuffer = device->beginSingleTimeCommands();

        vk::ImageMemoryBarrier barrier({},
        {
        },
        {
        },
        {
        }, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image,
                vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor,{}, 1, 0, 1));

        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

            commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                    vk::PipelineStageFlagBits::eTransfer,{},
            0, nullptr,
            0, nullptr,
            1, &barrier);

            vk::ImageBlit blit(vk::ImageSubresourceLayers(
                    vk::ImageAspectFlagBits::eColor, i - 1, 0, 1),{
                vk::Offset3D(0, 0, 0), vk::Offset3D(mipWidth, mipHeight, 1)
            },
            vk::ImageSubresourceLayers(
                    vk::ImageAspectFlagBits::eColor, i, 0, 1), {
                vk::Offset3D(0, 0, 0), vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1)
            });

            commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, 1, &blit, vk::Filter::eLinear);

            barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                    vk::PipelineStageFlagBits::eFragmentShader,{},
            0, nullptr, 0, nullptr, 1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,{},
        0, nullptr,
        0, nullptr,
        1, &barrier);


        device->endSingleTimeCommands(commandBuffer);

        this->imageView = device->createImageView(this->image, vk::Format::eR8G8B8A8Srgb,
                vk::ImageAspectFlagBits::eColor, this->mipLevels);

        vk::SamplerCreateInfo samplerInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
                vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
                0.0f, VK_TRUE, 16.0f, VK_FALSE, vk::CompareOp::eAlways,
                0.0f, static_cast<float> (mipLevels), vk::BorderColor::eIntOpaqueBlack, VK_FALSE);

        this->sampler = this->device.createSampler(samplerInfo);
    }

    Texture::~Texture() {
        this->device.destroy(this->sampler);
        this->device.destroy(this->imageView);
        this->device.destroy(this->image);
        this->device.free(this->imageMemory);
    }

    vk::DescriptorImageInfo Texture::getDescriptorImageInfo(uint32_t index) {
        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = this->imageView;
        imageInfo.sampler = this->sampler;

        return imageInfo;
    }


}