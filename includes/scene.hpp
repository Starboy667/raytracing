#pragma once

// #ifdef _WIN32
// #include <fileapi.h>
// #include <synchapi.h>
// #include <windows.h>
// #endif
// #include <Pathcch.h>
// #include <shlwapi.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <tchar.h>
// #define WIN32_LEAN_AND_MEAN
// #ifndef NOMINMAX
// #define NOMINMAX
// #endif

#include <glm.hpp>
#include <vector>

#include "yaml-cpp/yaml.h"
struct UniformBufferObject {
    alignas(16) glm::vec3 camera_forward;
    alignas(16) glm::vec3 camera_right;
    alignas(16) glm::vec3 camera_up;
    alignas(16) glm::vec3 camera_position;
    alignas(4) int sphereCount;
    alignas(4) uint32_t frameCount;
};

struct Sphere {
    alignas(16) glm::vec3 center;  // Aligned to 16 bytes
    alignas(4) float radius;       // Aligned to 4 bytes
    alignas(16) glm::vec3 color;   // Aligned to 16 bytes
    alignas(4) float padding;      // Add padding to ensure 16-byte alignment
};

namespace YAML {
template <>
struct convert<glm::vec3> {
    static Node encode(const glm::vec3& rhs) {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        return node;
    }
    static bool decode(const Node& node, glm::vec3& rhs) {
        if (!node.IsSequence() || node.size() != 3) return false;
        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        return true;
    }
};
template <>
struct convert<UniformBufferObject> {
    static Node encode(const UniformBufferObject& rhs) {
        Node node;
        node.push_back(rhs.camera_position);
        node.push_back(rhs.camera_forward);
        node.push_back(rhs.camera_right);
        node.push_back(rhs.camera_up);
        node.push_back(rhs.sphereCount);
        node.push_back(0);
        return node;
    }
    static bool decode(const Node& node, UniformBufferObject& rhs) {
        if (!node.IsSequence() || node.size() != 6) return false;
        rhs.camera_position = node[0].as<glm::vec3>();
        rhs.camera_forward = node[1].as<glm::vec3>();
        rhs.camera_right = node[2].as<glm::vec3>();
        rhs.camera_up = node[3].as<glm::vec3>();
        rhs.sphereCount = node[4].as<int>();
        rhs.frameCount = 0;
        // rhs.frameCount = node[5].as<uint32_t>();
        return true;
    }
};
template <>
struct convert<Sphere> {
    static Node encode(const Sphere& rhs) {
        Node node;
        node.push_back(rhs.center);
        node.push_back(rhs.radius);
        node.push_back(rhs.color);
        return node;
    }
    static bool decode(const Node& node, Sphere& rhs) {
        if (!node.IsSequence() || node.size() != 3) return false;
        rhs.center = node[0].as<glm::vec3>();
        rhs.radius = node[1].as<float>();
        rhs.color = node[2].as<glm::vec3>();
        return true;
    }
};
}  // namespace YAML

#include <iostream>
class Scene {
   public:
    Scene();
    ~Scene();
    const std::vector<Sphere>& spheres() const { return m_spheres; }
    const UniformBufferObject& camera() const { return m_camera; }
    void update(float dt) {
        m_camera.frameCount++;
        glm::vec3 old_position = m_camera.camera_position;
        velocity += acceleration * dt;
        m_camera.camera_position += velocity * dt;
        velocity = velocity * 0.98f;
        acceleration = acceleration * 0.98f;
        if (glm::length(old_position - m_camera.camera_position) > 0.01f) {
            m_camera.frameCount = 1;
        }
    }
    void reloadScene();
    void resetFrameCount() { m_camera.frameCount = 0; }
    void save();

    std::vector<Sphere> m_spheres;
    UniformBufferObject m_camera;
    float mouseSensitivity = 0.4f;
    float movementSpeed = 100.0f;
    glm::vec3 acceleration = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);
    float yaw = 90.0f;
    float pitch = 0.0f;
};