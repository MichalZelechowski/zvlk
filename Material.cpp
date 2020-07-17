/* 
 * File:   Material.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 12 lipca 2020, 01:08
 */

#include "Material.h"

namespace zvlk {

    Material::Material(zvlk::Device* device, std::shared_ptr<zvlk::Frame> frame, std::string name, glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, float shiness) :
    UniformBuffer(device, sizeof (MaterialUBO), frame) {
        this->name = name;
        this->ubos.resize(frame->getImagesNumber());
        for (uint32_t i = 0; i < frame->getImagesNumber(); ++i) {
            this->ubos[i].ambient = ambient;
            this->ubos[i].diffuse = diffuse;
            this->ubos[i].specular = specular;
            this->ubos[i].shiness = shiness;
            dynamic_cast<UniformBuffer*>(this)->update(i);
        }
    }

    void* Material::update(uint32_t index, float time) {
        return &this->ubos[index];
    }

    Material::~Material() {
    }

}