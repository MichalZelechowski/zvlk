/* 
 * File:   Texture.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 17 czerwca 2020, 13:02
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <string>

namespace zvlk {

    class Device;

    class Texture {
    public:
        Texture() = delete;
        Texture(const Texture& orig) = delete;

        Texture(Device* device, std::string texturePath);
        virtual ~Texture();

        VkDescriptorImageInfo getDescriptorBufferInfo(uint32_t index);
    private:
        VkDevice device;
        uint32_t mipLevels;
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory imageMemory;
        VkSampler sampler;
    };
}
#endif /* TEXTURE_H */

