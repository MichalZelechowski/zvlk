/* 
 * File:   VertexShader.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 18 czerwca 2020, 14:31
 */

#include "VertexShader.h"
#include "Model.h"

#include <iostream>
#include <vector>

namespace zvlk {

    VertexShader::~VertexShader() {
    }

    VertexShader::VertexShader(vk::Device device, const char* name) : Shader(device, name, vk::ShaderStageFlagBits::eVertex) {
        static vk::VertexInputBindingDescription bindingDescription = zvlk::Vertex::getBindingDescription();
        static std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions = zvlk::Vertex::getAttributeDescriptions();

        this->vertexInputInfo = vk::PipelineVertexInputStateCreateInfo({}, 1, &bindingDescription, 
                static_cast<uint32_t> (attributeDescriptions.size()), attributeDescriptions.data());
    }

    vk::PipelineVertexInputStateCreateInfo& VertexShader::getPipelineVertexInputStateCreateInfo() {
        return this->vertexInputInfo;
    }
}

