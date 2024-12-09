#include "scene.hpp"

#include "utils.hpp"

Scene::Scene() {
    m_camera.camera_position =
        glm::vec3(0.0f, 0.0f, -10.0f);                      // Move camera back
    m_camera.camera_forward = glm::vec3(0.0f, 0.0f, 1.0f);  // Looking along +Z
    m_camera.camera_right = glm::vec3(1.0f, 0.0f, 0.0f);    // Right along +X
    m_camera.camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_camera.sphereCount = 10;

    m_spheres.reserve(m_camera.sphereCount);
    for (int i = 0; i < m_camera.sphereCount; i++) {
        float x = random_float(-5.0f, 5.0f);
        float y = random_float(-5.0f, 5.0f);
        float z = random_float(-5.0f, 5.0f);
        float r = random_float();
        float g = random_float();
        float b = random_float();
        float radius = random_float(0.5f, 3.0f);

        Sphere sphere{};
        sphere.center = glm::vec3(x, y, z);
        sphere.radius = radius;
        sphere.color = glm::vec3(r, g, b);
        m_spheres.push_back(sphere);
    }
}

Scene::~Scene() {}
