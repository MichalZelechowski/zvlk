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

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    private:
        GLFWwindow *window;
        WindowCallback *callback;
    };

}
#endif /* WINDOW_H */

