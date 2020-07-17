/* 
 * File:   UniformBuffer.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 18 czerwca 2020, 17:02
 */

#ifndef UNIFORMBUFFER_H
#define UNIFORMBUFFER_H

#include <vulkan/vulkan.hpp>

#include "Device.h"
#include "Frame.h"

namespace zvlk {

    class UniformBuffer {
    public:
        UniformBuffer() = delete;
        UniformBuffer(const UniformBuffer& orig) = delete;
        UniformBuffer(zvlk::Device* device, vk::DeviceSize size, std::shared_ptr<zvlk::Frame> frame);
        virtual ~UniformBuffer();
        
        void create(std::shared_ptr<zvlk::Frame> frame);
        void destroy();
        void update(uint32_t index);
        vk::DeviceSize getSize();
        vk::DescriptorBufferInfo getDescriptorBufferInfo(uint32_t frame);
    protected:
        virtual void* update(uint32_t index, float time) = 0;
    private:
        zvlk::Device* device;
        vk::DeviceSize size;
        std::vector<vk::Buffer> uniformBuffers;
        std::vector<vk::DeviceMemory> uniformBuffersMemory;

    };
}

#endif /* UNIFORMBUFFER_H */

