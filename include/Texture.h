/* 
 * File:   Texture.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 17 czerwca 2020, 13:02
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include <vulkan/vulkan.hpp>

#include <string>

namespace zvlk {

    class Device;

    class Texture {
    public:
        Texture() = delete;
        Texture(const Texture& orig) = delete;

        Texture(Device* device, std::string texturePath);
        virtual ~Texture();

        vk::DescriptorImageInfo getDescriptorImageInfo(uint32_t index);
    private:
        vk::Device device;
        uint32_t mipLevels;
        vk::Image image;
        vk::ImageView imageView;
        vk::DeviceMemory imageMemory;
        vk::Sampler sampler;
    };
}
#endif /* TEXTURE_H */

