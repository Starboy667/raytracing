#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "device.hpp"
#include "scene.hpp"
#include "swap_chain.hpp"
class ComputePipeline {
   public:
    ComputePipeline(Device& device, SwapChain& swapChain, Scene& scene);
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
    void updateScene(uint32_t currentImage);

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
    const Scene& m_scene;
    // const std::vector<Sphere>& m_spheres;
    // const UniformBufferObject& m_camera;

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
