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
#include <glm/gtc/matrix_transform.hpp>

namespace zvlk {

    struct TransformationMatricesUBO {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    class TransformationMatrices : public UniformBuffer {
    public:
        TransformationMatrices() = delete;
        TransformationMatrices(const TransformationMatrices& orig) = delete;
        TransformationMatrices(zvlk::Device* device, zvlk::Frame* frame);
        virtual ~TransformationMatrices();
    protected:
        void* update(uint32_t index, float time);
    private:
        std::vector<TransformationMatricesUBO> ubos;
        zvlk::Frame* frame;
    };
}
#endif /* TRANSFORMATIONMATRICES_H */

