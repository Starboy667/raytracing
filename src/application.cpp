#include "application.hpp"

Application::Application(uint32_t width, uint32_t height, const char* name) {
    InitWindow(width, height, name);
    _engine = std::make_unique<Engine>(width, height, _window);
}

Application::~Application() {}

void Application::Run() {
    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();
        _engine->render();
        // calculateFrameRate();
        double currentTime = glfwGetTime();
        lastFrameTime = (currentTime - lastTime) * 1000.0;
        lastTime = currentTime;
    }
}

void Application::InitWindow(uint32_t width, uint32_t height,
                             const char* name) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    _window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    glfwSetWindowUserPointer(_window, this);
    glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
}
