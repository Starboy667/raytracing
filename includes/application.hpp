#pragma once

#include <memory>

#include "../src/engine/engine.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Application {
   public:
    Application(uint32_t width, uint32_t height, const char* name);
    ~Application();
    void Run();

   private:
    void Init();
    void Shutdown();
    void InitWindow(uint32_t width, uint32_t height, const char* title);
    static void framebufferResizeCallback(GLFWwindow* window, int width,
                                          int height) {
        auto app =
            reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->_engine->SetFramebufferResized(true);
    }

   public:
    std::unique_ptr<Engine> _engine;

   private:
    GLFWwindow* _window;
};
Application CreateApplication(int argc, char** argv);