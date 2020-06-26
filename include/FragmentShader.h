/* 
 * File:   FragmentShader.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 18 czerwca 2020, 14:46
 */

#ifndef FRAGMENTSHADER_H
#define FRAGMENTSHADER_H

#include <vulkan/vulkan.hpp>

#include "Shader.h"

namespace zvlk {

    class FragmentShader : public Shader {
    public:
        FragmentShader() = delete;
        FragmentShader(const FragmentShader& orig) = delete;
        FragmentShader(vk::Device device, const char* name);
        virtual ~FragmentShader();
    private:

    };
}
#endif /* FRAGMENTSHADER_H */

