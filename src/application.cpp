#include "application.hpp"

#include <sstream>

Application::Application(uint32_t width, uint32_t height, const char* name) {
    InitWindow(width, height, name);
    m_scene = Scene();
    _engine = std::make_unique<Engine>(width, height, _window, m_scene);
}

Application::~Application() { m_scene.save(); }

void Application::Run() {
    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();
        if (glfwGetKey(_window, GLFW_KEY_R) == GLFW_PRESS) {
            m_scene.reloadScene();
        }
        m_scene.update();
        _engine->render();
        showFPS(_window);
    }
}

void Application::showFPS(GLFWwindow* pWindow) {
    double currentTime = glfwGetTime();
    double delta = currentTime - lastTime;
    nbFrames++;
    if (delta >= 1.0) {
        // If last cout was more than 1 sec ago
        // std::cout << 1000.0 / double(nbFrames) << std::endl;

        double fps = double(nbFrames) / delta;
        std::stringstream ss;
        ss << "Raytracer" << " " << " [" << fps << " FPS]";
        glfwSetWindowTitle(pWindow, ss.str().c_str());
        nbFrames = 0;
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
