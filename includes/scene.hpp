#pragma once

#include <glm/glm.hpp>
#include <vector>
struct UniformBufferObject {
    alignas(16) glm::vec3 camera_forward;
    alignas(16) glm::vec3 camera_right;
    alignas(16) glm::vec3 camera_up;
    alignas(16) glm::vec3 camera_position;
    alignas(4) int sphereCount;
};

struct Sphere {
    alignas(16) glm::vec3 center;  // Aligned to 16 bytes
    alignas(4) float radius;       // Aligned to 4 bytes
    alignas(16) glm::vec3 color;   // Aligned to 16 bytes
    alignas(4) float padding;      // Add padding to ensure 16-byte alignment
};

class Scene {
   public:
    Scene();
    ~Scene();
    const std::vector<Sphere>& spheres() const { return m_spheres; }
    const UniformBufferObject& camera() const { return m_camera; }
    void update() {  }

   private:
    std::vector<Sphere> m_spheres;
    UniformBufferObject m_camera;
};