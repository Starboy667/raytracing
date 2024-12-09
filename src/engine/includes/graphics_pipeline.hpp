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
class GraphicsPipeline {
   public:
    GraphicsPipeline(Device& device, SwapChain& swapChain, Instance& instance,
                     GLFWwindow* window);
    ~GraphicsPipeline();

    void render();
    void setFramebufferResized(bool resized) { m_framebufferResized = resized; }

   private:
    void initImGui();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer,
                             uint32_t imageIndex);
    void createSyncObjects();

   private:
    Device& m_device;
    SwapChain& m_swapChain;
    Instance& m_instance;
    GLFWwindow* m_window;

    VkDescriptorPool m_descriptorPool;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_framebuffers;
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    bool m_framebufferResized = false;
    uint32_t m_currentFrame = 0;
};