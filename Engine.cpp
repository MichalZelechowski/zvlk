/* 
 * File:   Engine.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 18 czerwca 2020, 21:02
 */

#include <vector>
#include <stdexcept>

#include "Engine.h"

namespace zvlk {

    Engine::~Engine() {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            this->device.destroy(renderFinishedSemaphores[i]);
            this->device.destroy(imageAvailableSemaphores[i]);
            this->device.destroy(inFlightFences[i]);
        }

        this->deviceObject->freeCommandBuffers(commandBuffers);

        for (ExecutionUnit& unit : this->units) {
            this->device.destroy(unit.graphicsPipeline);
            this->device.destroy(unit.pipelineLayout);
            this->device.destroy(unit.descriptorPool);
            this->device.destroy(unit.descriptorSetLayout);
        }
    }

    Engine::Engine(zvlk::Frame* frame, zvlk::Device* deviceObject) {
        this->device = deviceObject->getGraphicsDevice();
        this->frameNumber = frame->getImagesNumber();
        this->frame = frame;
        this->deviceObject = deviceObject;

        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(this->frameNumber, vk::Fence());

        vk::SemaphoreCreateInfo semaphoreInfo({});
        vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            imageAvailableSemaphores[i] = this->device.createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = this->device.createSemaphore(semaphoreInfo);
            inFlightFences[i] = this->device.createFence(fenceInfo);
        }
    }

    void Engine::enableShaders(VertexShader& vertexShader, FragmentShader& fragmentShader) {
        this->units.push_back({vertexShader, fragmentShader, nullptr, nullptr,
            {}});
    }

    void Engine::draw(Model& model, TransformationMatrices& transformationMatrices, Texture& texture) {
        if (this->units.empty()) {
            throw std::runtime_error("drawing with no shaders enabled");
        }

        ExecutionUnit& unit = this->units.back();
        unit.models.push_back({model, texture, transformationMatrices,
            {}});
    }

    void Engine::compile() {
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);
        vk::Viewport viewport(0.0f, 0.0f, (float) frame->getWidth(), (float) frame->getHeight(), 0.0f, 1.0f);
        vk::Rect2D scissor(vk::Offset2D(0, 0), vk::Extent2D(frame->getWidth(), frame->getHeight()));
        vk::PipelineViewportStateCreateInfo viewportState({}, 1, &viewport, 1, &scissor);
        vk::PipelineRasterizationStateCreateInfo rasterizer({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill,
                vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
        vk::PipelineMultisampleStateCreateInfo multisampling({}, deviceObject->getMaxUsableSampleCount(),
                VK_TRUE, 1.0f, nullptr, VK_FALSE, VK_FALSE);

        vk::PipelineColorBlendAttachmentState colorBlendAttachment(VK_FALSE, vk::BlendFactor::eOne, vk::BlendFactor::eZero,
                vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        vk::PipelineColorBlendStateCreateInfo colorBlending({}, VK_FALSE, vk::LogicOp::eCopy, 1, &colorBlendAttachment,{0.0f, 0.0f, 0.0f, 0.0f});
        vk::PipelineDepthStencilStateCreateInfo depthStencil({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE,{},
        {
        }, 0.0f, 1.0f);

        int i = 0;
        for (ExecutionUnit& unit : this->units) {
            vk::PipelineShaderStageCreateInfo shaderStages[] = {
                unit.vertexShader.getPipelineShaderStageCreateInfo(),
                unit.fragmentShader.getPipelineShaderStageCreateInfo()
            };

            vk::DescriptorSetLayoutBinding uboLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
            vk::DescriptorSetLayoutBinding samplerLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
            std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

            vk::DescriptorSetLayoutCreateInfo layoutInfo({}, static_cast<uint32_t> (bindings.size()), bindings.data());
            unit.descriptorSetLayout = this->device.createDescriptorSetLayout(layoutInfo);

            std::array<vk::DescriptorPoolSize, 2> poolSizes = {
                vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, this->frameNumber),
                vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, this->frameNumber)
            };
            vk::DescriptorPoolCreateInfo poolInfo({}, this->frameNumber, poolSizes.size(), poolSizes.data());
            unit.descriptorPool = this->device.createDescriptorPool(poolInfo);

            std::vector<vk::DescriptorSetLayout> layouts(this->frameNumber, unit.descriptorSetLayout);
            vk::DescriptorSetAllocateInfo allocInfo(unit.descriptorPool, this->frameNumber, layouts.data());

            unit.pipelineLayout = this->device.createPipelineLayout(vk::PipelineLayoutCreateInfo({}, 1, &unit.descriptorSetLayout));
            vk::PipelineVertexInputStateCreateInfo& iscr = unit.vertexShader.getPipelineVertexInputStateCreateInfo();

            vk::GraphicsPipelineCreateInfo pipelineInfo({}, 2, shaderStages, &iscr, &inputAssembly,{}, &viewportState,
                    &rasterizer, &multisampling, &depthStencil, &colorBlending,{}, unit.pipelineLayout,
                    frame->getRenderPass(), 0, vk::Pipeline(), -1);

            unit.graphicsPipeline = device.createGraphicsPipelines(vk::PipelineCache(), {pipelineInfo})[0];

            for (ModelUnit& model : unit.models) {
                model.descriptorSets = this->device.allocateDescriptorSets(allocInfo);
                for (size_t j = 0; j < this->frameNumber; j++) {
                    vk::DescriptorBufferInfo matrixInfo = model.matrix.getDescriptorBufferInfo(j);
                    vk::DescriptorImageInfo textureInfo = model.texture.getDescriptorBufferInfo(j);

                    std::array<vk::WriteDescriptorSet, 2> descriptorWrites = {
                        vk::WriteDescriptorSet(model.descriptorSets[j],
                        0, 0, 1, vk::DescriptorType::eUniformBuffer,
                        {}, &matrixInfo,
                        {}),
                        vk::WriteDescriptorSet(model.descriptorSets[j], 1, 0, 1, vk::DescriptorType::eCombinedImageSampler, &textureInfo,
                        {},
                        {})
                    };

                    this->device.updateDescriptorSets(descriptorWrites, {});
                }
            }

            i++;
        }

        this->deviceObject->allocateCommandBuffers(this->frameNumber, this->commandBuffers);

        for (size_t i = 0; i < this->commandBuffers.size(); i++) {
            this->commandBuffers[i].begin(vk::CommandBufferBeginInfo());
            vk::RenderPassBeginInfo renderPassInfo = this->frame->getRenderPassBeginInfo(i);
            //attachmets, like depth buffer and color frame are attached
            commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

            for (ExecutionUnit& unit : this->units) {
                commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, unit.graphicsPipeline);

                for (ModelUnit& model : unit.models) {
                    //objects are bound
                    //this can be optimized for a range of models
                    vk::Buffer vertexBuffers[] = {model.model.getVertexBuffer()};
                    vk::DeviceSize offsets[] = {0};
                    commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
                    commandBuffers[i].bindIndexBuffer(model.model.getIndexBuffer(), 0, vk::IndexType::eUint32);
                    commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, unit.pipelineLayout, 0, 1, &model.descriptorSets[i], 0, nullptr);
                    //objects are drawn
                    commandBuffers[i].drawIndexed(model.model.getNumberOfIndices(), 1, 0, 0, 0);
                    commandBuffers[i].endRenderPass();
                }
            }
            commandBuffers[i].end();
        }
    }

    void Engine::execute() {
        this->device.waitForFences(1, &this->inFlightFences[this->currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        vk::Result result = this->device.acquireNextImageKHR(this->frame->getShwapChain(), UINT64_MAX,
                this->imageAvailableSemaphores[this->currentFrame], vk::Fence(), &imageIndex);

        if (result == vk::Result::eErrorOutOfDateKHR) {
            //recreateSwapChain();
            std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
            return;
        } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        for (EngineCallback* callback : this->callbacks) {
            callback->update(imageIndex);
        }

        if (this->imagesInFlight[imageIndex]) {
            device.waitForFences(1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }

        imagesInFlight[imageIndex] = inFlightFences[currentFrame];
        device.resetFences(1, &inFlightFences[currentFrame]);
        
        vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        vk::SubmitInfo submitInfo(1, waitSemaphores, waitStages, 1, &commandBuffers[imageIndex], 1, signalSemaphores);
        this->deviceObject->submitGraphics(&submitInfo, inFlightFences[currentFrame]);

        vk::SwapchainKHR swapChains[] = {this->frame->getShwapChain()};
        vk::PresentInfoKHR presentInfo(1, signalSemaphores, 1, swapChains, &imageIndex);
        result = this->deviceObject->present(&presentInfo);

        if (result == vk::Result::eErrorOutOfDateKHR|| result == vk::Result::eSuboptimalKHR) {
            //|| framebufferResized) {
            std::cout << "VK_SUBOPTIMAL_KHR" << std::endl;
            //framebufferResized = false;
            //recreateSwapChain();
            //throw std::runtime_error("result on present is wrong!");
        } else if (result != vk::Result::eSuccess) {
            std::cout << "Swap chain other" << std::endl;
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}
