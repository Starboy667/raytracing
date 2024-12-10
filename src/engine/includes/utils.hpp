#pragma once

#include <fstream>

#include "device.hpp"

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
uint32_t findMemoryType(Device& device, uint32_t typeFilter,
                        VkMemoryPropertyFlags properties);
