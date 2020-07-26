/* 
 * File:   Material.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 12 lipca 2020, 01:08
 */

#ifndef MATERIAL_H
#define MATERIAL_H

#include "UniformBuffer.h"
#include "Texture.h"
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
        Material(zvlk::Device* device, std::shared_ptr<zvlk::Frame> frame, std::string name, glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, float shiness, std::string diffuseTextureName);
        virtual ~Material();
        
        vk::DescriptorImageInfo getDescriptorImageInfo(uint32_t index);
    protected:
        virtual void* update(uint32_t index, float time);
    private:
        std::vector<MaterialUBO> ubos;
        std::string name;
        zvlk::Texture* diffuseTexture;
    };

}

#endif /* MATERIAL_H */

