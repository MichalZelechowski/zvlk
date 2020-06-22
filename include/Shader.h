/* 
 * File:   Shader.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 18 czerwca 2020, 14:47
 */

#ifndef SHADER_H
#define SHADER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace zvlk {

    class Shader {
    protected:
        Shader() = delete;
        Shader(const Shader& orig) = delete;
        Shader(VkDevice device, const char* name, VkShaderStageFlagBits stage);
        virtual ~Shader();
        
    public:
        VkPipelineShaderStageCreateInfo& getPipelineShaderStageCreateInfo();
    protected:
        VkDevice device;
        VkShaderModule shaderModule;
        VkPipelineShaderStageCreateInfo shaderStageInfo;
    };
}
#endif /* SHADER_H */

