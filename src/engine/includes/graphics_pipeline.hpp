#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../includes/device.hpp"
#include "../includes/device_structures.hpp"
#include "../includes/instance.hpp"
#include "../includes/swap_chain.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "scene.hpp"
class GraphicsPipeline {
   public:
    GraphicsPipeline(Device& device, SwapChain& swapChain, Instance& instance,
                     GLFWwindow* window, Scene& scene);
    ~GraphicsPipeline();

    void render(uint32_t imageIndex, uint32_t currentFrame);
    VkCommandBuffer* getCurrentCommandBuffer(uint32_t currentFrame) {
        return &m_commandBuffers[currentFrame];
    }

   private:
    void initImGui();
    void createCommandPool();
    void createCommandBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer,
                             uint32_t imageIndex);

   private:
    Device& m_device;
    SwapChain& m_swapChain;
    Instance& m_instance;
    GLFWwindow* m_window;

    Scene& m_scene;
    VkDescriptorPool m_descriptorPool;
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
};