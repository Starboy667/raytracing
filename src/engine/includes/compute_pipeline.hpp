#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "includes/device.hpp"
#include "includes/swap_chain.hpp"

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

class ComputePipeline {
   public:
    ComputePipeline(Device& device, SwapChain& swapChain);
    ~ComputePipeline();
    void render();
    void setFramebufferResized(bool resized) { m_framebufferResized = resized; }

   private:
    void createPipeline();
    void createDescriptorSetLayout();
    void createCommandPool();
    void createCommandBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createSyncObjects();
    void createUniformBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer,
                             uint32_t imageIndex);
    void updateUniformBuffer(uint32_t currentImage);

   private:
    Device& m_device;
    SwapChain& m_swapChain;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;

    // uniforms
    std::vector<VkDescriptorSet> m_descriptorSets;
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;

    // sphere and scene buffer
    std::vector<VkBuffer> m_sphereBuffers;
    std::vector<VkDeviceMemory> m_sphereBuffersMemory;
    std::vector<void*> m_sphereBuffersMapped;
    // TODO: bouger
    std::vector<Sphere> m_spheres;
    UniformBufferObject m_camera;

    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
    // std::vector<VkCommandBuffer> m_computeCommandBuffers;
    VkDescriptorPool m_descriptorPool;

    // sync
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    uint32_t m_currentFrame = 0;
    bool m_framebufferResized = false;
};
