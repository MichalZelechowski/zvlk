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

    TransformationMatrices::TransformationMatrices(zvlk::Device* device, std::shared_ptr<zvlk::Frame> frame) : UniformBuffer(device, sizeof (TransformationMatricesUBO), frame) {
        this->ubos.resize(frame->getImagesNumber());
        this->current = glm::mat4(1.0f);
    }

    void* TransformationMatrices::update(uint32_t index, float time) {
        this->ubos[index].model = glm::mat4(this->current);
        return &this->ubos[index];
    }

    TransformationMatrices& TransformationMatrices::rotate(float angleDegrees, glm::vec3 direction) {
        this->current = glm::rotate(glm::mat4(1.0f), glm::radians(angleDegrees), direction) * this->current;
        return *this;
    }

    TransformationMatrices& TransformationMatrices::translate(glm::vec3 vector) {
        this->current = glm::translate(glm::mat4(1.0f), vector) * this->current;
        return *this;
    }

}

