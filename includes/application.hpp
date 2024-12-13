#pragma once

// #include <memory>

#include "../src/engine/engine.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <random>

#include "scene.hpp"
class Application {
   public:
    Application(uint32_t width, uint32_t height, const char* name);
    ~Application();
    void Run();

   private:
    void showFPS(GLFWwindow* pWindow);
    void InitWindow(uint32_t width, uint32_t height, const char* title);
    static void framebufferResizeCallback(GLFWwindow* window, int width,
                                          int height) {
        auto app =
            reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->_engine->setFramebufferResized(true);
    }
    void handleInput();

   public:
    std::unique_ptr<Engine> _engine;

   private:
    GLFWwindow* _window;
    Scene m_scene;

    glm::vec2 m_lastMousePosition = glm::vec2(0.0f);

    float dt = 0.0f;
    double lastTime = 0.0f;
    size_t nbFrames = 0;
    bool m_cursorEnabled = false;
    bool m_escapePressed = false;
};