#include "scene.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "utils.hpp"

Scene::Scene() {
    std::ifstream file(std::filesystem::current_path().string() +
                       "/../res/scenes/scene.yaml");
    YAML::Node scene = YAML::Load(file);
    m_spheres = scene["spheres"].as<std::vector<Sphere>>();
    m_camera = scene["camera"].as<UniformBufferObject>();

    // m_camera.camera_position = glm::vec3(0.0f, 0.0f, -30.0f);
    // m_camera.camera_forward = glm::vec3(0.0f, 0.0f, 1.0f);
    // m_camera.camera_right = glm::vec3(1.0f, 0.0f, 0.0f);
    // m_camera.camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    // m_camera.sphereCount = 40;
    // m_camera.frameCount = 0;

    // m_spheres.reserve(m_camera.sphereCount);
    // for (int i = 0; i < m_camera.sphereCount; i++) {
    //     float x = random_float(-10.0f, 10.0f);
    //     float y = random_float(-10.0f, 10.0f);
    //     float z = random_float(-10.0f, 10.0f);
    //     float r = random_float();
    //     float g = random_float();
    //     float b = random_float();
    //     float radius = random_float(0.5f, 3.0f);

    //     Sphere sphere{};
    //     sphere.center = glm::vec3(x, y, z);
    //     sphere.radius = radius;
    //     sphere.color = glm::vec3(r, g, b);
    //     m_spheres.push_back(sphere);
    // }
    // // m_spheres[0].color = glm::vec3(0.8f, 0.5f, 0.2f);
    // m_spheres[0].color = glm::vec3(1.0f, 0.0f, 0.0f);
    // m_spheres[0].center = glm::vec3(0.0f, -20.0f, 0.0f);
    // m_spheres[0].radius = 10.0f;
    // m_spheres[1].color = glm::vec3(0.0f, 1.0f, 0.0f);
    // m_spheres[1].center = glm::vec3(20.0f, 20.0f, 0.0f);
    // m_spheres[1].radius = 10.0f;
    // m_spheres[2].color = glm::vec3(0.0f, 0.0f, 1.0f);
    // m_spheres[2].center = glm::vec3(-20.0f, 20.0f, 0.0f);
    // m_spheres[2].radius = 10.0f;
}

void Scene::save() {
    YAML::Emitter out;
    out << YAML::BeginMap;
    // save spheres
    out << YAML::Key << "spheres" << YAML::Value
        << YAML::convert<std::vector<Sphere>>::encode(m_spheres);
    // save camera
    out << YAML::Key << "camera" << YAML::Value
        << YAML::convert<UniformBufferObject>::encode(m_camera);
    out << YAML::EndSeq << YAML::EndMap;
    std::string path =
        std::filesystem::current_path().string() + "/../res/scenes/scene.yaml";
    std::ofstream fout(path);
    fout << out.c_str();
    fout.close();
}

Scene::~Scene() { m_spheres.clear(); }