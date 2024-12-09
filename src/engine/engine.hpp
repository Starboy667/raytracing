
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "includes/compute_pipeline.hpp"
#include "includes/device.hpp"
#include "includes/instance.hpp"
#include "includes/swap_chain.hpp"
class Engine {
   public:
    Engine(uint32_t width, uint32_t height, GLFWwindow* window);
    ~Engine() { cleanup(); };
    void render() { m_computePipeline->render(); }
    void setFramebufferResized(bool resized) {
        m_computePipeline->setFramebufferResized(resized);
    }

   private:
    void initVulkan();
    void cleanup();

   private:
    GLFWwindow* m_window;

    std::unique_ptr<Instance> m_instance;
    std::unique_ptr<Device> m_device;
    std::unique_ptr<SwapChain> m_swapChain;
    std::unique_ptr<ComputePipeline> m_computePipeline;
};
