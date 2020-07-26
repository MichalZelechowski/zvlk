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
        alignas(16) glm::vec3 eye;
        alignas(16) glm::vec3 center;
    };

    class Camera : public UniformBuffer {
    public:
        Camera() = delete;
        Camera(const Camera& orig) = delete;
        Camera(zvlk::Device* device, std::shared_ptr<zvlk::Frame> frame, glm::vec3 eye, glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f), float fov = 45.0f,
                glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float near = 0.1f, float far = 10.0f);
        virtual ~Camera();

        virtual void* update(uint32_t index, float time);
        
        Camera& rotateEye(float angle, glm::vec3 axis=glm::vec3(0.0f, 1.0f, 0.0f));
        Camera& translateEye(glm::vec3 vector);
    private:
        glm::vec3 eye;
        glm::vec3 center;
        float fov;
        glm::vec3 up;
        float near;
        float far;

        std::shared_ptr<zvlk::Frame> frame;
        std::vector<CameraUBO> ubos;
    };
}
#endif /* CAMERA_H */

