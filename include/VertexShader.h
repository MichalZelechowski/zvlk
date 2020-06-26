/* 
 * File:   VertexShader.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 18 czerwca 2020, 14:31
 */

#ifndef VERTEXSHADER_H
#define VERTEXSHADER_H
#include <vulkan/vulkan.hpp>

#include "Shader.h"

namespace zvlk {

    class VertexShader : public Shader {
    public:
        VertexShader() = delete;
        VertexShader(const VertexShader& orig) = delete;
        VertexShader(VkDevice device, const char* name);
        virtual ~VertexShader();
        
        vk::PipelineVertexInputStateCreateInfo& getPipelineVertexInputStateCreateInfo();
    private:
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    };
}
#endif /* VERTEXSHADER_H */

