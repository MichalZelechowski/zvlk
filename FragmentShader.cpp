/* 
 * File:   FragmentShader.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 18 czerwca 2020, 14:46
 */

#include "FragmentShader.h"
#include "Model.h"

namespace zvlk {

    FragmentShader::~FragmentShader() {
    }

    FragmentShader::FragmentShader(VkDevice device, const char* name) : Shader(device, name, VK_SHADER_STAGE_FRAGMENT_BIT) {
    }
}
