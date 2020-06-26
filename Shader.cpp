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

    Shader::Shader(vk::Device device, const char* name, vk::ShaderStageFlagBits stage) {
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

        this->shaderModule = this->device.createShaderModule(vk::ShaderModuleCreateInfo({}, buffer.size(), reinterpret_cast<const uint32_t*> (buffer.data())));
        this->shaderStageInfo = vk::PipelineShaderStageCreateInfo({}, stage, this->shaderModule, "main");
    }

    Shader::~Shader() {
        this->device.destroy(this->shaderModule);
    }
    
    vk::PipelineShaderStageCreateInfo& Shader::getPipelineShaderStageCreateInfo() {
        return this->shaderStageInfo;
    }
}

