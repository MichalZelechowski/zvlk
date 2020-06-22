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
        for (size_t i = 0; i < uniformBuffers.size(); i++) {
            this->device->freeMemory(uniformBuffers[i], uniformBuffersMemory[i]);
        }
    }

    UniformBuffer::UniformBuffer(zvlk::Device* device, VkDeviceSize size, zvlk::Frame* frame) {
        this->size = size;
        this->device = device;

        uniformBuffers.resize(frame->getImagesNumber());
        uniformBuffersMemory.resize(frame->getImagesNumber());

        for (size_t i = 0; i < frame->getImagesNumber(); i++) {
            device->createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
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

    VkDeviceSize UniformBuffer::getSize() {
        return this->size;
    }

    VkDescriptorBufferInfo UniformBuffer::getDescriptorBufferInfo( uint32_t index) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = this->uniformBuffers[index];
        bufferInfo.offset = 0;
        bufferInfo.range = this->getSize();
        return bufferInfo;
    }

}