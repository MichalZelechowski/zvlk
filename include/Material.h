/* 
 * File:   Material.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 12 lipca 2020, 01:08
 */

#ifndef MATERIAL_H
#define MATERIAL_H

#include "UniformBuffer.h"
#include <glm/vec4.hpp>
#include <vector>

namespace zvlk {

    struct MaterialUBO {
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
        alignas(16) float shiness;
    };

    class Material : public UniformBuffer {
    public:
        Material() = delete;
        Material(const Material& orig) = delete;
        Material(zvlk::Device* device, zvlk::Frame* frame, std::string name, glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, float shiness);
        virtual ~Material();
    protected:
        virtual void* update(uint32_t index, float time);
    private:
        std::vector<MaterialUBO> ubos;
        std::string name;
    };

}

#endif /* MATERIAL_H */

