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

    VertexShader::VertexShader(VkDevice device, const char* name) : Shader(device, name, VK_SHADER_STAGE_VERTEX_BIT) {
        static VkVertexInputBindingDescription bindingDescription = zvlk::Vertex::getBindingDescription();
        static std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = zvlk::Vertex::getAttributeDescriptions();

        this->vertexInputInfo = {};
        this->vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        this->vertexInputInfo.vertexBindingDescriptionCount = 1;
        this->vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
        this->vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t> (attributeDescriptions.size());
        this->vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional
    }

    VkPipelineVertexInputStateCreateInfo& VertexShader::getPipelineVertexInputStateCreateInfo() {
        return this->vertexInputInfo;
    }
}

