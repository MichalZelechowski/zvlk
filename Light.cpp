/* 
 * File:   Light.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 13 lipca 2020, 22:49
 */

#include "Light.h"

namespace zvlk {

    Lights::Lights(zvlk::Device* device, zvlk::Frame* frame) : UniformBuffer(device, sizeof (LightsUBO), frame) {
        this->ubos.resize(frame->getImagesNumber());
    }

    Lights::~Lights() {
        for (Light* light: this->lights) {
            delete light;
        }
    }

    void* Lights::update(uint32_t index, float time) {
        this->ubos[index].numberOfLights = static_cast<float>(this->lights.size());
        for (size_t i = 0; i<this->ubos[index].numberOfLights; ++i) {
            this->ubos[index].lights[i] = {this->lights[i]->position, this->lights[i]->color, this->lights[i]->attenuation};
        }
        return &this->ubos[index];
    }

    Light::Light(glm::vec3 position, glm::vec4 color, float attenuation) {
        this->position = position;
        this->color = color;
        this->attenuation = attenuation;
    }

    Light::~Light() {
    }
}
