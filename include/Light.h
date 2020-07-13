/* 
 * File:   Light.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 13 lipca 2020, 22:49
 */

#ifndef LIGHT_H
#define LIGHT_H

#include "UniformBuffer.h"
#define MAX_LIGHTS 8

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace zvlk {

    struct LightUBO {
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec4 color;
        alignas(16) float attenuation;
    };
    
    struct LightsUBO {
        alignas(16) uint32_t numberOfLights;
        alignas(16) LightUBO lights[MAX_LIGHTS];
    };
    
    class Light {
    public:
        Light() = delete;
        Light(const Light& orig) = delete;
        Light(glm::vec3 position, glm::vec4 color, float attenuation);
        virtual ~Light();

    private:
        glm::vec3 position;
        glm::vec4 color;
        float attenuation;
        friend class Lights; 
    };

    class Lights : public UniformBuffer {
    public:
        Lights() = delete;
        Lights(const Lights& orig) = delete;
        Lights(zvlk::Device* device, zvlk::Frame* frame);
        virtual ~Lights();
        
        void addLight(zvlk::Light* light) {
            if (this->lights.size() == MAX_LIGHTS) {
                throw std::runtime_error("Maximum number of lights reached");
            }
            this->lights.push_back(light);
        }
    protected:
        virtual void* update(uint32_t index, float time);
    private:
        std::vector<zvlk::Light*> lights;
        std::vector<LightsUBO> ubos;
    };

}
#endif /* LIGHT_H */

