/* 
 * File:   Window.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 11 czerwca 2020, 15:16
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>
#include <string>

namespace zvlk {

    class WindowCallback {
    public:
        virtual void resize(int width, int height) = 0;
        virtual void key(int key, int action, int mods) = 0;

    };

    class Window {
    public:
        Window(int width, int height, const std::string title, WindowCallback* callback);
        Window(const Window& orig) = delete;
        virtual ~Window();

        void waitResize();
        std::tuple<int, int> getSize();
        bool isClosed();
        GLFWwindow *getWindow() const;
        void toggleFullscreen();
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    private:
        GLFWwindow *window;
        WindowCallback *callback;
    };

}
#endif /* WINDOW_H */

