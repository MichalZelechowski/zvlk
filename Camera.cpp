/* 
 * File:   Camera.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 6 lipca 2020, 23:01
 */

#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace zvlk {

    Camera::~Camera() {
    }

    Camera::Camera(zvlk::Device* device, zvlk::Frame* frame, glm::vec3 eye, glm::vec3 center, float fov, glm::vec3 up, float near, float far) :
    UniformBuffer(device, sizeof (CameraUBO), frame), eye(eye), center(center), fov(fov), up(up), near(near), far(far) {
        this->ubos.resize(frame->getImagesNumber());
        this->frame = frame;
    }

    void* Camera::update(uint32_t index, float time) {
        this->ubos[index].view = glm::lookAt(eye, center, up);
        this->ubos[index].proj = glm::perspective(fov, this->frame->getWidth() / (float) this->frame->getHeight(), near, far);
        this->ubos[index].proj[1][1] *= -1;

        return &this->ubos[index];
    }
}
