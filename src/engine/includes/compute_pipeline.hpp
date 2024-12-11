#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "device.hpp"
#include "scene.hpp"
#include "swap_chain.hpp"
class ComputePipeline {
   public:
    ComputePipeline(Device& device, SwapChain& swapChain, Scene& scene);
    ~ComputePipeline();
    void render(uint32_t imageIndex, uint32_t currentFrame);
    VkCommandBuffer* getCurrentCommandBuffer(uint32_t currentFrame) {
        return &m_commandBuffers[currentFrame];
    }
    void windowResized();

   private:
    void createPipeline();
    void createDescriptorSetLayout();
    void createCommandPool();
    void createCommandBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createUniformBuffers();
    void createAccumulationImage();
    void recordCommandBuffer(VkCommandBuffer commandBuffer,
                             uint32_t currentFrame, uint32_t imageIndex);
    void updateScene(uint32_t currentImage);
    void updateDescriptorSets(uint32_t imageIndex, uint32_t currentFrame);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

   private:
    Device& m_device;
    SwapChain& m_swapChain;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;

    // accumulation image
    VkImage m_accumulationImage;
    VkDeviceMemory m_accumulationImageMemory;
    VkImageView m_accumulationImageView;

    // uniforms
    std::vector<VkDescriptorSet> m_descriptorSets;
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;

    // sphere and scene buffer
    std::vector<VkBuffer> m_sphereBuffers;
    std::vector<VkDeviceMemory> m_sphereBuffersMemory;
    std::vector<void*> m_sphereBuffersMapped;
    Scene& m_scene;

    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
    VkDescriptorPool m_descriptorPool;
};
