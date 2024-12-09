
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

#include "includes/compute_pipeline.hpp"
#include "includes/config.hpp"
#include "includes/device.hpp"
#include "includes/graphics_pipeline.hpp"
#include "includes/instance.hpp"
#include "includes/swap_chain.hpp"
class Engine {
   public:
    Engine(uint32_t width, uint32_t height, GLFWwindow* window, Scene& scene);
    ~Engine() { cleanup(); };
    void render();
    void setFramebufferResized(bool resized) { m_framebufferResized = resized; }

   private:
    void initVulkan(Scene& scene);
    void cleanup();
    void createSyncObjects() {
        m_imageAvailableSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(config::MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(m_device->device(), &semaphoreInfo, nullptr,
                                  &m_imageAvailableSemaphores[i]) !=
                    VK_SUCCESS ||
                vkCreateSemaphore(m_device->device(), &semaphoreInfo, nullptr,
                                  &m_renderFinishedSemaphores[i]) !=
                    VK_SUCCESS ||
                vkCreateFence(m_device->device(), &fenceInfo, nullptr,
                              &m_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error(
                    "failed to create graphics synchronization objects for a"
                    "frame!");
            }
        }
    }

   private:
    GLFWwindow* m_window;
    std::unique_ptr<Instance> m_instance;
    std::unique_ptr<Device> m_device;
    std::unique_ptr<SwapChain> m_swapChain;
    std::unique_ptr<GraphicsPipeline> m_graphicsPipeline;
    std::unique_ptr<ComputePipeline> m_computePipeline;

    bool m_framebufferResized = false;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    uint32_t m_currentFrame = 0;
};
