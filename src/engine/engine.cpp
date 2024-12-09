#include "engine.hpp"

#include "includes/compute_pipeline.hpp"
#include "includes/config.hpp"
#include "includes/device_structures.hpp"
#include "includes/instance.hpp"
#include "includes/swap_chain.hpp"
#include "includes/utils.hpp"

Engine::Engine(uint32_t width, uint32_t height, GLFWwindow* window,
               Scene& scene)
    : m_window(window) {
    initVulkan(scene);
}

void Engine::initVulkan(Scene& scene) {
    m_instance = std::make_unique<Instance>(m_window);
    m_device = std::make_unique<Device>(m_instance->getInstance(),
                                        m_instance->getSurface());
    m_swapChain = std::make_unique<SwapChain>(*m_device, m_window,
                                              m_instance->getSurface());
    m_computePipeline =
        std::make_unique<ComputePipeline>(*m_device, *m_swapChain, scene);
    m_graphicsPipeline = std::make_unique<GraphicsPipeline>(
        *m_device, *m_swapChain, *m_instance, m_window);
}

void Engine::cleanup() {
    vkDeviceWaitIdle(m_device->device());
    m_swapChain.reset();
    m_computePipeline.reset();
    m_graphicsPipeline.reset();
    m_device.reset();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Engine::render() {
    m_computePipeline->render();
    // m_graphicsPipeline->render();
}
