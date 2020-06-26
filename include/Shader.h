/* 
 * File:   Shader.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 18 czerwca 2020, 14:47
 */

#ifndef SHADER_H
#define SHADER_H

#include <vulkan/vulkan.hpp>

namespace zvlk {

    class Shader {
    protected:
        Shader() = delete;
        Shader(const Shader& orig) = delete;
        Shader(vk::Device device, const char* name, vk::ShaderStageFlagBits stage);
        virtual ~Shader();
        
    public:
        vk::PipelineShaderStageCreateInfo& getPipelineShaderStageCreateInfo();
    protected:
        vk::Device device;
        vk::ShaderModule shaderModule;
        vk::PipelineShaderStageCreateInfo shaderStageInfo;
    };
}
#endif /* SHADER_H */

