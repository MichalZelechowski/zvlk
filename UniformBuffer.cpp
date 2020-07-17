/* 
 * File:   UniformBuffer.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 18 czerwca 2020, 17:02
 */

#include "UniformBuffer.h"

#include <chrono>

namespace zvlk {

    UniformBuffer::~UniformBuffer() {
        this->destroy();
    }
    
    void UniformBuffer::destroy() {
        for (size_t i = 0; i < uniformBuffers.size(); i++) {
            this->device->freeMemory(uniformBuffers[i], uniformBuffersMemory[i]);
        }
        uniformBuffers.clear();
        uniformBuffersMemory.clear();
    }

    UniformBuffer::UniformBuffer(zvlk::Device* device, vk::DeviceSize size, std::shared_ptr<zvlk::Frame> frame) {
        this->size = size;
        this->device = device;
        this->create(frame);
    }
    
    void UniformBuffer::create(std::shared_ptr<zvlk::Frame> frame) {
        uniformBuffers.resize(frame->getImagesNumber());
        uniformBuffersMemory.resize(frame->getImagesNumber());

        for (size_t i = 0; i < frame->getImagesNumber(); i++) {
            device->createBuffer(size, vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                    uniformBuffers[i], uniformBuffersMemory[i]);
        }
    }

    void UniformBuffer::update(uint32_t index) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        void* ubo = this->update(index, time);

        this->device->copyMemory(this->size, ubo, this->uniformBuffersMemory[index]);
    }

    vk::DeviceSize UniformBuffer::getSize() {
        return this->size;
    }

    vk::DescriptorBufferInfo UniformBuffer::getDescriptorBufferInfo( uint32_t index) {
        vk::DescriptorBufferInfo bufferInfo(this->uniformBuffers[index], 0, this->getSize());
        return bufferInfo;
    }

}