/* 
 * File:   VertexShader.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 18 czerwca 2020, 14:31
 */

#ifndef VERTEXSHADER_H
#define VERTEXSHADER_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "Shader.h"

namespace zvlk {

    class VertexShader : public Shader {
    public:
        VertexShader() = delete;
        VertexShader(const VertexShader& orig) = delete;
        VertexShader(VkDevice device, const char* name);
        virtual ~VertexShader();
        
        VkPipelineVertexInputStateCreateInfo& getPipelineVertexInputStateCreateInfo();
    private:
        VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    };
}
#endif /* VERTEXSHADER_H */

