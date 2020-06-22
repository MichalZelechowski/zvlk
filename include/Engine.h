/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Engine.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 18 czerwca 2020, 21:02
 */

#ifndef ENGINE_H
#define ENGINE_H

#include <list>

#include "VertexShader.h"
#include "FragmentShader.h"
#include "Model.h"
#include "Texture.h"
#include "TransformationMatrices.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

namespace zvlk {

    typedef struct ModelUnit {
        zvlk::Model& model;
        zvlk::Texture& texture;
        zvlk::TransformationMatrices& matrix;
        std::vector<VkDescriptorSet> descriptorSets;
    } ModelUnit;

    typedef struct ExecutionUnit {
        zvlk::VertexShader& vertexShader;
        zvlk::FragmentShader& fragmentShader;
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        std::list<ModelUnit> models;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;
    } ExecutionUnit;

    class EngineCallback {
    public:
        virtual void update(uint32_t frameIndex) = 0;
    };

    class Engine {
    public:
        Engine() = delete;
        Engine(const Engine& orig) = delete;
        Engine(zvlk::Frame* frame, zvlk::Device* deviceObject);
        virtual ~Engine();

        inline void addCallback(EngineCallback* callback) {
            this->callbacks.push_back(callback);
        };

        void enableShaders(zvlk::VertexShader& vertexShader, zvlk::FragmentShader& fragmentShader);
        void draw(zvlk::Model& model, zvlk::TransformationMatrices& transformationMatrices, zvlk::Texture& texture);
        void compile();
        void execute();
    private:
        std::list<ExecutionUnit> units;
        std::vector<VkCommandBuffer> commandBuffers;
        VkDevice device;
        uint32_t frameNumber;
        zvlk::Frame* frame;
        zvlk::Device* deviceObject;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        std::list<EngineCallback*> callbacks;
        size_t currentFrame = 0;
    };
}
#endif /* ENGINE_H */

