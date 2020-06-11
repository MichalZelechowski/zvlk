/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Window.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 11 czerwca 2020, 15:16
 */

#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

#include "WindowCallback.h"

namespace zvlk {

    class Window {
    public:
        Window(int width, int height, const std::string title, WindowCallback* callback);
        Window(const Window& orig) = delete;
        virtual ~Window();
        
        void waitResize();
        std::tuple<int, int> getSize();
        bool isClosed();
        GLFWwindow *getWindow() const;

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    private:
        GLFWwindow *window;
        WindowCallback *callback;
    };

}
#endif /* WINDOW_H */

