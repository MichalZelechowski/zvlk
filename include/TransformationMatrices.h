/* 
 * File:   TransformationMatrices.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 18 czerwca 2020, 17:41
 */

#ifndef TRANSFORMATIONMATRICES_H
#define TRANSFORMATIONMATRICES_H

#include "UniformBuffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace zvlk {

    struct TransformationMatricesUBO {
        glm::mat4 model;
    };

    class TransformationMatrices : public UniformBuffer {
    public:
        TransformationMatrices() = delete;
        TransformationMatrices(const TransformationMatrices& orig) = delete;
        TransformationMatrices(zvlk::Device* device, std::shared_ptr<zvlk::Frame> frame);
        virtual ~TransformationMatrices();
        
        TransformationMatrices& rotate(float angleDegrees, glm::vec3 direction);
        TransformationMatrices& translate(glm::vec3 vector);
    protected:
        void* update(uint32_t index, float time);
    private:
        std::vector<TransformationMatricesUBO> ubos;
        glm::mat4 current;
    };
}
#endif /* TRANSFORMATIONMATRICES_H */

