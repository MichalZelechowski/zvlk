/* 
 * File:   Engine.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 18 czerwca 2020, 21:02
 */

#ifndef ENGINE_H
#define ENGINE_H
#include <vulkan/vulkan.hpp>

#include <list>

#include "VertexShader.h"
#include "FragmentShader.h"
#include "Model.h"
#include "Texture.h"
#include "TransformationMatrices.h"
#include "Camera.h"
#include "Light.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

namespace zvlk {

    typedef struct ModelUnit {
        zvlk::Model& model;
        zvlk::Texture& texture;
        zvlk::TransformationMatrices& matrix;
        vk::DescriptorPool descriptorPool;
        std::vector<vk::DescriptorSet> descriptorSets;
    } ModelUnit;

    typedef struct ExecutionUnit {
        zvlk::VertexShader& vertexShader;
        zvlk::FragmentShader& fragmentShader;
        vk::DescriptorPool descriptorPool;
        std::vector<vk::DescriptorSet> descriptorSets;
        std::list<ModelUnit> models;
        vk::Pipeline graphicsPipeline;
    } ExecutionUnit;

    class EngineCallback {
    public:
        virtual void update(uint32_t frameIndex) = 0;
    };

    class Engine {
    public:
        Engine() = delete;
        Engine(const Engine& orig) = delete;
        Engine(std::shared_ptr<zvlk::Frame> frame, zvlk::Device* deviceObject);
        virtual ~Engine();

        void clean();

        inline void addCallback(EngineCallback* callback) {
            this->callbacks.push_back(callback);
        };

        inline void setCamera(zvlk::Camera *camera) {
            this->camera = camera;
        }

        inline void attachLight(zvlk::Light* light) {
            this->lights->addLight(light);
            for (uint32_t i = 0; i < this->frameNumber; ++i) {
                dynamic_cast<zvlk::UniformBuffer*>(this->lights)->update(i);
            }
        }

        void enableShaders(zvlk::VertexShader& vertexShader, zvlk::FragmentShader& fragmentShader);
        void draw(zvlk::Model& model, zvlk::TransformationMatrices& transformationMatrices, zvlk::Texture& texture);
        void compile();
        vk::Bool32 execute(vk::Bool32 framebufferResized);
    private:
        std::list<ExecutionUnit> units;
        std::vector<vk::CommandBuffer> commandBuffers;
        vk::Device device;
        uint32_t frameNumber;
        std::shared_ptr<zvlk::Frame> frame;
        zvlk::Device* deviceObject;
        zvlk::Camera* camera;
        zvlk::Lights* lights;

        vk::DescriptorSetLayout sceneLayout;
        vk::DescriptorSetLayout modelLayout;
        vk::DescriptorSetLayout materialLayout;
        vk::DescriptorPool descriptorPool;
        std::vector<vk::DescriptorSet> descriptorSets;
        vk::PipelineLayout pipelineLayout;

        std::vector<vk::Semaphore> imageAvailableSemaphores;
        std::vector<vk::Semaphore> renderFinishedSemaphores;
        std::vector<vk::Fence> inFlightFences;
        std::vector<vk::Fence> imagesInFlight;
        std::list<EngineCallback*> callbacks;
        size_t currentFrame = 0;
    };
}
#endif /* ENGINE_H */

