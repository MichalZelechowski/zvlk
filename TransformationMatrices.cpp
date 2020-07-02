/* 
 * File:   TransformationMatrices.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 18 czerwca 2020, 17:41
 */

#include "TransformationMatrices.h"

namespace zvlk {

    TransformationMatrices::~TransformationMatrices() {
    }

    TransformationMatrices::TransformationMatrices(zvlk::Device* device, zvlk::Frame* frame) : UniformBuffer(device, sizeof (TransformationMatricesUBO), frame) {
        this->ubos.resize(frame->getImagesNumber());
        this->frame = frame;
    }

    void* TransformationMatrices::update(uint32_t index, float time) {
        this->ubos[index].model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        this->ubos[index].model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * this->ubos[index].model;
        this->ubos[index].view = glm::lookAt(glm::vec3(1000.0f, 1000.0f, 1000.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        this->ubos[index].proj = glm::perspective(glm::radians(45.0f), this->frame->getWidth() / (float) this->frame->getHeight(), 0.1f, 2000.0f);
        this->ubos[index].proj[1][1] *= -1;


        return &this->ubos[index];
    }
}

