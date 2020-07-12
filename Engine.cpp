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
        this->clean();
    }

    void Engine::clean() {
        this->deviceObject->freeCommandBuffers(commandBuffers);

        for (ExecutionUnit& unit : this->units) {
            this->device.destroy(unit.graphicsPipeline);
            this->device.destroy(unit.descriptorPool);
            unit.descriptorSets.clear();
            for (ModelUnit& model : unit.models) {
                model.descriptorSets.clear();
                this->device.destroy(model.descriptorPool);
            }
        }
        this->descriptorSets.clear();
        this->device.destroy(this->pipelineLayout);
        this->device.destroy(this->descriptorPool);

        this->device.destroy(this->sceneLayout);
        this->device.destroy(this->modelLayout);
        this->device.destroy(this->materialLayout);
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
        this->units.push_back({vertexShader, fragmentShader,
            nullptr,
            {},
            {}, nullptr});
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
                vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
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

        //per scene
        vk::DescriptorSetLayoutBinding cameraBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
        //per model
        vk::DescriptorSetLayoutBinding transformationBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
        //per material
        vk::DescriptorSetLayoutBinding samplerLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
        vk::DescriptorSetLayoutBinding materialBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);

        std::array<vk::DescriptorSetLayoutBinding, 4> descriptorSetLayoutBindings = {cameraBinding, transformationBinding, samplerLayoutBinding, materialBinding};

        vk::DescriptorSetLayoutCreateInfo sceneLayoutInfo({}, 1, &descriptorSetLayoutBindings.data()[0]);
        this->sceneLayout = this->device.createDescriptorSetLayout(sceneLayoutInfo);
        vk::DescriptorSetLayoutCreateInfo modelLayoutInfo({}, 1, &descriptorSetLayoutBindings.data()[1]);
        this->modelLayout = this->device.createDescriptorSetLayout(modelLayoutInfo);
        vk::DescriptorSetLayoutCreateInfo materialLayoutInfo({}, 2, &descriptorSetLayoutBindings.data()[2]);
        this->materialLayout = this->device.createDescriptorSetLayout(materialLayoutInfo);

        vk::DescriptorPoolSize scenePoolSize(vk::DescriptorType::eUniformBuffer, this->frameNumber);
        vk::DescriptorPoolCreateInfo poolInfo({}, this->frameNumber, 1, &scenePoolSize);
        this->descriptorPool = this->device.createDescriptorPool(poolInfo);

        std::vector<vk::DescriptorSetLayout> layouts(this->frameNumber, this->sceneLayout);
        vk::DescriptorSetAllocateInfo allocInfo(this->descriptorPool, this->frameNumber, layouts.data());
        this->descriptorSets = this->device.allocateDescriptorSets(allocInfo);

        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {this->sceneLayout, this->modelLayout, this->materialLayout};
        this->pipelineLayout = this->device.createPipelineLayout(vk::PipelineLayoutCreateInfo({}, descriptorSetLayouts.size(), descriptorSetLayouts.data()));

        std::vector<vk::WriteDescriptorSet> descriptorWrites;
        std::vector<vk::DescriptorBufferInfo> cameraInfos(this->frameNumber);
        for (size_t j = 0; j < this->frameNumber; j++) {
            cameraInfos[j] = this->camera->getDescriptorBufferInfo(j);
            descriptorWrites.push_back(vk::WriteDescriptorSet(this->descriptorSets[j],
                    0, 0, 1, vk::DescriptorType::eUniformBuffer,{}, &cameraInfos[j],{}));
        }
        this->device.updateDescriptorSets(descriptorWrites,{});

        for (ExecutionUnit& unit : this->units) {
            vk::PipelineShaderStageCreateInfo shaderStages[] = {
                unit.vertexShader.getPipelineShaderStageCreateInfo(),
                unit.fragmentShader.getPipelineShaderStageCreateInfo()
            };

            vk::PipelineVertexInputStateCreateInfo& iscr = unit.vertexShader.getPipelineVertexInputStateCreateInfo();

            vk::GraphicsPipelineCreateInfo pipelineInfo({}, 2, shaderStages, &iscr, &inputAssembly,{}, &viewportState,
                    &rasterizer, &multisampling, &depthStencil, &colorBlending,{}, this->pipelineLayout,
                    frame->getRenderPass(), 0, vk::Pipeline(), -1);

            unit.graphicsPipeline = device.createGraphicsPipelines(vk::PipelineCache(),{pipelineInfo})[0];

            vk::DescriptorPoolSize modelPoolSize(vk::DescriptorType::eUniformBuffer, this->frameNumber * unit.models.size());
            vk::DescriptorPoolCreateInfo poolInfo({}, this->frameNumber * unit.models.size(), 1, &modelPoolSize);
            unit.descriptorPool = this->device.createDescriptorPool(poolInfo);

            std::vector<vk::DescriptorSetLayout> layouts(this->frameNumber * unit.models.size(), this->modelLayout);
            vk::DescriptorSetAllocateInfo allocInfo(unit.descriptorPool, layouts.size(), layouts.data());
            unit.descriptorSets = this->device.allocateDescriptorSets(allocInfo);

            std::vector<vk::WriteDescriptorSet> descriptorWrites;
            std::vector<vk::DescriptorBufferInfo> matrixInfos(this->frameNumber * unit.models.size());
            for (size_t j = 0; j < this->frameNumber; j++) {
                int i = 0;
                for (ModelUnit& model : unit.models) {
                    uint32_t index = i * this->frameNumber + j;
                    matrixInfos[index] = model.matrix.getDescriptorBufferInfo(j);
                    descriptorWrites.push_back(vk::WriteDescriptorSet(unit.descriptorSets[index],
                            0, 0, 1, vk::DescriptorType::eUniformBuffer,{
                    }, &matrixInfos[index],{}));
                    i++;
                }
            }
            this->device.updateDescriptorSets(descriptorWrites,{});

            for (ModelUnit& model : unit.models) {
                uint32_t partsCount = model.model.getPartNames().size();
                vk::DescriptorPoolSize samplerPoolSize(vk::DescriptorType::eCombinedImageSampler, this->frameNumber * partsCount);
                vk::DescriptorPoolSize materialPoolSize(vk::DescriptorType::eUniformBuffer, this->frameNumber * partsCount);
                vk::DescriptorPoolSize modelPoolSizes[] = {samplerPoolSize, materialPoolSize};
                vk::DescriptorPoolCreateInfo poolInfo({}, this->frameNumber * partsCount, 2, modelPoolSizes);
                model.descriptorPool = this->device.createDescriptorPool(poolInfo);

                std::vector<vk::DescriptorSetLayout> layouts(this->frameNumber * partsCount, this->materialLayout);
                vk::DescriptorSetAllocateInfo allocInfo(model.descriptorPool, layouts.size(), layouts.data());
                model.descriptorSets = this->device.allocateDescriptorSets(allocInfo);

                std::vector<vk::WriteDescriptorSet> partWrites;
                //so far there's one texture info per model, in the future there should be one texture info per part
                std::vector<vk::DescriptorBufferInfo> materialInfos(this->frameNumber * partsCount);
                std::vector<vk::DescriptorImageInfo> texturesInfos(this->frameNumber * partsCount);
                for (size_t j = 0; j < this->frameNumber; j++) {
                    for (uint32_t i = 0; i < partsCount; ++i) {
                        uint32_t index = i * this->frameNumber + j;
                        zvlk::Material* mat = model.model.getMaterial(i);
                        materialInfos[index] = mat->getDescriptorBufferInfo(j);
                        texturesInfos[index] = model.texture.getDescriptorBufferInfo(j);
                        partWrites.push_back(
                                vk::WriteDescriptorSet(model.descriptorSets[index], 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &texturesInfos[index],{},
                        {
                        }));
                        partWrites.push_back(
                                vk::WriteDescriptorSet(model.descriptorSets[index], 1, 0, 1, vk::DescriptorType::eUniformBuffer,{}, &materialInfos[index],{
                        }));
                    }
                }
                this->device.updateDescriptorSets(partWrites,{});
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
                commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->pipelineLayout, 0, 1, &this->descriptorSets[i], 0, nullptr);

                int j = 0;
                for (ModelUnit& model : unit.models) {
                    commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->pipelineLayout, 1, 1, &unit.descriptorSets[j * this->frameNumber + i], 0, nullptr);
                    // if parts are sorted by material, this could be optimized and only some bindings done for this descriptor sets
                    int k = 0;
                    for (std::string& name : model.model.getPartNames()) {
                        commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->pipelineLayout, 2, 1, &model.descriptorSets[k * this->frameNumber + i], 0, nullptr);

                        vk::Buffer vertexBuffers[] = {model.model.getVertexBuffer(name)};
                        vk::DeviceSize offsets[] = {0};
                        commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
                        commandBuffers[i].bindIndexBuffer(model.model.getIndexBuffer(name), 0, vk::IndexType::eUint32);
                        commandBuffers[i].drawIndexed(model.model.getNumberOfIndices(name), 1, 0, 0, 0);
                        k++;
                    }
                    j++;
                }
            }
            commandBuffers[i].endRenderPass();
            commandBuffers[i].end();
        }
    }

    vk::Bool32 Engine::execute(vk::Bool32 framebufferResized) {
        this->device.waitForFences(1, &this->inFlightFences[this->currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        vk::Result result = this->device.acquireNextImageKHR(this->frame->getSwapChain(), UINT64_MAX,
                this->imageAvailableSemaphores[this->currentFrame], vk::Fence(), &imageIndex);

        if (result == vk::Result::eErrorOutOfDateKHR) {
            return false;
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

        vk::SwapchainKHR swapChains[] = {this->frame->getSwapChain()};
        vk::PresentInfoKHR presentInfo(1, signalSemaphores, 1, swapChains, &imageIndex);
        result = this->deviceObject->present(&presentInfo);

        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
            return false;
        } else if (result != vk::Result::eSuccess) {
            std::cout << "Swap chain other" << std::endl;
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        return true;
    }
}
