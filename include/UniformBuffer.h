/* 
 * File:   UniformBuffer.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 18 czerwca 2020, 17:02
 */

#ifndef UNIFORMBUFFER_H
#define UNIFORMBUFFER_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "Device.h"
#include "Frame.h"

namespace zvlk {

    class UniformBuffer {
    public:
        UniformBuffer() = delete;
        UniformBuffer(const UniformBuffer& orig) = delete;
        UniformBuffer(zvlk::Device* device, VkDeviceSize size, zvlk::Frame* frame);
        virtual ~UniformBuffer();

        void update(uint32_t index);
        VkDeviceSize getSize();
        VkDescriptorBufferInfo getDescriptorBufferInfo(uint32_t frame);
    protected:
        virtual void* update(uint32_t index, float time) = 0;
    private:
        zvlk::Device* device;
        VkDeviceSize size;
        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;
        VkWriteDescriptorSet writeDescriptorSet;

    };
}

#endif /* UNIFORMBUFFER_H */

