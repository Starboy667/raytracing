#include "application.hpp"

#include <sstream>

Application::Application(uint32_t width, uint32_t height, const char* name) {
    InitWindow(width, height, name);
    m_scene = Scene();
    _engine = std::make_unique<Engine>(width, height, _window, m_scene);

    // Initialize camera forward vector
    m_scene.m_camera.camera_forward.x =
        cos(glm::radians(m_scene.yaw)) * cos(glm::radians(m_scene.pitch));
    m_scene.m_camera.camera_forward.y = sin(glm::radians(m_scene.pitch));
    m_scene.m_camera.camera_forward.z =
        sin(glm::radians(m_scene.yaw)) * cos(glm::radians(m_scene.pitch));
    m_scene.m_camera.camera_forward =
        glm::normalize(m_scene.m_camera.camera_forward);

    // Recalculate the right and up vectors to keep them orthogonal
    m_scene.m_camera.camera_right = glm::normalize(glm::cross(
        m_scene.m_camera.camera_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_scene.m_camera.camera_up = glm::normalize(glm::cross(
        m_scene.m_camera.camera_right, m_scene.m_camera.camera_forward));
}

Application::~Application() { m_scene.save(); }

void Application::Run() {
    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();
        handleInput();
        showFPS(_window);
        m_scene.update(dt);
        _engine->render();
    }
}

void Application::handleInput() {
    if (glfwGetKey(_window, GLFW_KEY_R) == GLFW_PRESS) {
        m_scene.reloadScene();
    }

    // Keyboard camera movement
    if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS) {
        m_scene.acceleration +=
            m_scene.m_camera.camera_forward * m_scene.movementSpeed;
    }
    if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS) {
        m_scene.acceleration -=
            m_scene.m_camera.camera_forward * m_scene.movementSpeed;
    }
    if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS) {
        m_scene.acceleration -=
            m_scene.movementSpeed * m_scene.m_camera.camera_right;
    }
    if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS) {
        m_scene.acceleration +=
            m_scene.movementSpeed * m_scene.m_camera.camera_right;
    }
    if (glfwGetKey(_window, GLFW_KEY_E) == GLFW_PRESS) {
        m_scene.acceleration -=
            m_scene.m_camera.camera_up * m_scene.movementSpeed;
    }
    if (glfwGetKey(_window, GLFW_KEY_Q) == GLFW_PRESS) {
        m_scene.acceleration +=
            m_scene.m_camera.camera_up * m_scene.movementSpeed;
    }

    // Mouse camera movement
    if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        glm::dvec2 mousePosition = glm::vec2(0.0f);
        glfwGetCursorPos(_window, &mousePosition.x, &mousePosition.y);
        glm::vec2 mouseDelta = (glm::vec2)mousePosition - m_lastMousePosition;
        m_scene.yaw += mouseDelta.x * m_scene.mouseSensitivity;
        m_scene.pitch += mouseDelta.y * m_scene.mouseSensitivity;

        // Clamp pitch to prevent the camera from flipping over
        if (m_scene.pitch > 89.0f) m_scene.pitch = 89.0f;
        if (m_scene.pitch < -89.0f) m_scene.pitch = -89.0f;

        m_scene.m_camera.camera_forward.x =
            cos(glm::radians(m_scene.yaw)) * cos(glm::radians(m_scene.pitch));
        m_scene.m_camera.camera_forward.y = sin(glm::radians(m_scene.pitch));
        m_scene.m_camera.camera_forward.z =
            sin(glm::radians(m_scene.yaw)) * cos(glm::radians(m_scene.pitch));
        m_scene.m_camera.camera_forward =
            glm::normalize(m_scene.m_camera.camera_forward);

        // Recalculate the right and up vectors to keep them orthogonal
        m_scene.m_camera.camera_right = glm::normalize(glm::cross(
            m_scene.m_camera.camera_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_scene.m_camera.camera_up = glm::normalize(glm::cross(
            m_scene.m_camera.camera_right, m_scene.m_camera.camera_forward));
        m_lastMousePosition = mousePosition;
        m_scene.m_camera.frameCount = 0;
    }
    glm::dvec3 tmp;
    glfwGetCursorPos(_window, &tmp.x, &tmp.y);
    m_lastMousePosition = glm::vec2(tmp.x, tmp.y);
}

void Application::showFPS(GLFWwindow* pWindow) {
    double currentTime = glfwGetTime();
    dt = (float)(currentTime - lastTime);
    nbFrames++;
    // if (dt >= 1.0) {
    //     // If last cout was more than 1 sec ago
    //     // std::cout << 1000.0 / double(nbFrames) << std::endl;

    //     double fps = double(nbFrames) / dt;
    //     std::stringstream ss;
    //     ss << "Raytracer" << " " << " [" << fps << " FPS]";
    //     glfwSetWindowTitle(pWindow, ss.str().c_str());
    //     nbFrames = 0;
    // }
    lastTime = currentTime;
}
void Application::InitWindow(uint32_t width, uint32_t height,
                             const char* name) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    _window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    glfwSetWindowUserPointer(_window, this);
    glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
}
