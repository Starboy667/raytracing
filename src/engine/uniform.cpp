#include "includes/uniform.hpp"

#include <random>

#include "includes/config.hpp"

void Engine::createUniformBuffers() {
    VkDeviceSize sphereBufferSize = sizeof(Sphere) * m_camera.sphereCount;
    m_sphereBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
    m_sphereBuffersMemory.resize(config::MAX_FRAMES_IN_FLIGHT);
    m_sphereBuffersMapped.resize(config::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(sphereBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_sphereBuffers[i], m_sphereBuffersMemory[i]);
        vkMapMemory(m_device->device(), m_sphereBuffersMemory[i], 0,
                    sphereBufferSize, 0, &m_sphereBuffersMapped[i]);
    }
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniformBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
    m_uniformBuffersMemory.resize(config::MAX_FRAMES_IN_FLIGHT);
    m_uniformBuffersMapped.resize(config::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_uniformBuffers[i], m_uniformBuffersMemory[i]);
        vkMapMemory(m_device->device(), m_uniformBuffersMemory[i], 0,
                    bufferSize, 0, &m_uniformBuffersMapped[i]);
    }
}

void Engine::updateUniformBuffer(uint32_t currentImage) {
    memcpy(m_uniformBuffersMapped[currentImage], &m_camera, sizeof(m_camera));
    memcpy(m_sphereBuffersMapped[currentImage], m_spheres.data(),
           m_camera.sphereCount * sizeof(Sphere));
}

void Engine::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(config::MAX_FRAMES_IN_FLIGHT,
                                               m_computeDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(config::MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();
    m_descriptorSets.resize(config::MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_device->device(), &allocInfo,
                                 m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
        // Storage Image descriptor (binding = 0)
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = m_swapChain->imageViews()[i];
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageInfo.sampler = nullptr;

        // Storage Buffer descriptor for spheres (binding = 1)
        VkDescriptorBufferInfo sphereBufferInfo{};
        sphereBufferInfo.buffer = m_sphereBuffers[i];
        sphereBufferInfo.offset = 0;
        sphereBufferInfo.range = sizeof(Sphere) * m_spheres.size();

        // Uniform Buffer descriptor (binding = 2)
        VkDescriptorBufferInfo uniformBufferInfo{};
        uniformBufferInfo.buffer = m_uniformBuffers[i];
        uniformBufferInfo.offset = 0;
        uniformBufferInfo.range = sizeof(UniformBufferObject);

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        // Storage Image (binding = 0)
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &imageInfo;

        // Sphere Storage Buffer (binding = 1)
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &sphereBufferInfo;

        // Uniform Buffer (binding = 2)
        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = m_descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &uniformBufferInfo;

        try {
            vkUpdateDescriptorSets(
                m_device->device(),
                static_cast<uint32_t>(descriptorWrites.size()),
                descriptorWrites.data(), 0, nullptr);
        } catch (const std::exception& e) {
            std::cerr << "Error updating descriptor sets: " << e.what()
                      << std::endl;
            throw std::runtime_error("failed to update descriptor sets!");
        }
    }
}
