#pragma once

#include <fstream>
#include <random>

#include "includes/device.hpp"
#include "includes/utils.hpp"
inline float random_float() {
    static std::random_device rd;
    static std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    static std::mt19937 generator(rd());
    return distribution(generator);
}

inline float random_float(float min, float max) {
    return min + random_float() * (max - min);
}

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void createBuffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, VkBuffer& buffer,
                  VkDeviceMemory& bufferMemory);
VkShaderModule createShaderModule(VkDevice device,
                                  const std::vector<char>& code);