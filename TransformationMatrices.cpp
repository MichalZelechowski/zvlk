/* 
 * File:   TransformationMatrices.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 18 czerwca 2020, 17:41
 */

#include "TransformationMatrices.h"

#include <glm/gtc/matrix_transform.hpp>

namespace zvlk {

    TransformationMatrices::~TransformationMatrices() {
    }

    TransformationMatrices::TransformationMatrices(zvlk::Device* device, zvlk::Frame* frame) : UniformBuffer(device, sizeof (TransformationMatricesUBO), frame) {
        this->ubos.resize(frame->getImagesNumber());
    }

    void* TransformationMatrices::update(uint32_t index, float time) {
        this->ubos[index].model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        this->ubos[index].model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -250.0f, 0.0f)) * this->ubos[index].model;
        this->ubos[index].model = glm::rotate(glm::mat4(1.0f), 0.0f * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * this->ubos[index].model;

        return &this->ubos[index];
    }
    
}

