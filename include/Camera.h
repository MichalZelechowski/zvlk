/* 
 * File:   Camera.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 6 lipca 2020, 23:01
 */

#ifndef CAMERA_H
#define CAMERA_H

#include "UniformBuffer.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace zvlk {

    struct CameraUBO {
        glm::mat4 view;
        glm::mat4 proj;
    };

    class Camera : public UniformBuffer {
    public:
        Camera() = delete;
        Camera(const Camera& orig) = delete;
        Camera(zvlk::Device* device, zvlk::Frame* frame, glm::vec3 eye, glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f), float fov = 45.0f,
                glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f), float near = 0.1f, float far = 10.0f);
        virtual ~Camera();

        virtual void* update(uint32_t index, float time);
        
        void rotateEye(float angle, glm::vec3 axis=glm::vec3(0.0f, 0.0f, 1.0f));
    private:
        glm::vec3 eye;
        glm::vec3 center;
        float fov;
        glm::vec3 up;
        float near;
        float far;

        zvlk::Frame* frame;
        std::vector<CameraUBO> ubos;
    };
}
#endif /* CAMERA_H */

