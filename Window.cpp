/* 
 * File:   Window.cpp
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 * 
 * Created on 11 czerwca 2020, 15:16
 */

#include "Window.h"
#include <tuple>

namespace zvlk {

    Window::Window(int width, int height, const std::string title, WindowCallback* callback) {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        this->window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
        this->callback = callback;

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetKeyCallback(window, keyCallback);
    }

    Window::~Window() {
        glfwDestroyWindow(this->window);

        glfwTerminate();
    }

    void Window::toggleFullscreen() {
        GLFWmonitor *currentMonitor = glfwGetWindowMonitor(this->window);
        if (currentMonitor) {
            std::tuple<int,int> size = this->getSize();
            const GLFWvidmode* mode = glfwGetVideoMode(currentMonitor);
            glfwSetWindowMonitor(this->window, nullptr, 0, 0, std::get<0>(size), std::get<1>(size), mode->refreshRate);
        } else {
            int monitorCount;
            GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
            const GLFWvidmode* mode = glfwGetVideoMode(monitors[0]);

            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            glfwSetWindowMonitor(this->window, monitors[0], 0, 0, mode->width, mode->height, mode->refreshRate);
        }
    }

    void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        Window* vlkWindow = reinterpret_cast<Window*> (glfwGetWindowUserPointer(window));

        vlkWindow->callback->resize(width, height);
    }

    void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Window* vlkWindow = reinterpret_cast<Window*> (glfwGetWindowUserPointer(window));

        vlkWindow->callback->key(key, action, mods);
    }

    void Window::waitResize() {
        int width = 0, height = 0;
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(this->window, &width, &height);
            glfwWaitEvents();
        }
    }

    std::tuple<int, int> Window::getSize() {
        int width, height;
        glfwGetFramebufferSize(this->window, &width, &height);
        return std::make_tuple(width, height);
    }

    bool Window::isClosed() {
        return glfwWindowShouldClose(this->window);
    }

    GLFWwindow *Window::getWindow() const {
        return this->window;
    }

}