/* 
 * File:   Shader.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 18 czerwca 2020, 14:47
 */

#include "Shader.h"

#include <iostream>
#include <vector>
#include <fstream>

namespace zvlk {

    Shader::Shader(VkDevice device, const char* name, VkShaderStageFlagBits stage) {
        this->device = device;

        std::ifstream file(name, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = buffer.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*> (buffer.data());

        if (vkCreateShaderModule(device, &createInfo, nullptr, &this->shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        this->shaderStageInfo = {};
        this->shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        this->shaderStageInfo.stage = stage;
        this->shaderStageInfo.module = this->shaderModule;
        this->shaderStageInfo.pName = "main";
    }

    Shader::~Shader() {
        vkDestroyShaderModule(this->device, this->shaderModule, nullptr);
    }
    
    VkPipelineShaderStageCreateInfo& Shader::getPipelineShaderStageCreateInfo() {
        return this->shaderStageInfo;
    }
}

