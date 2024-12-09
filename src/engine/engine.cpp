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
    createSyncObjects();
    m_swapChain = std::make_unique<SwapChain>(*m_device, m_window,
                                              m_instance->getSurface());
    m_computePipeline =
        std::make_unique<ComputePipeline>(*m_device, *m_swapChain, scene);
    m_graphicsPipeline = std::make_unique<GraphicsPipeline>(
        *m_device, *m_swapChain, *m_instance, m_window, scene);
}

void Engine::cleanup() {
    vkDeviceWaitIdle(m_device->device());
    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_device->device(), m_renderFinishedSemaphores[i],
                           nullptr);
        vkDestroySemaphore(m_device->device(), m_imageAvailableSemaphores[i],
                           nullptr);
        vkDestroyFence(m_device->device(), m_inFlightFences[i], nullptr);
    }

    m_swapChain.reset();
    m_computePipeline.reset();
    m_graphicsPipeline.reset();
    m_device.reset();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Engine::render() {
    // Wait for previous frame to complete
    vkWaitForFences(m_device->device(), 1, &m_inFlightFences[m_currentFrame],
                    VK_TRUE, UINT64_MAX);

    // Acquire next image
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        m_device->device(), m_swapChain->getSwapChain(), UINT64_MAX,
        m_imageAvailableSemaphores[m_currentFrame],  // Signal when image is
                                                     // available
        VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        m_swapChain->recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Reset fence only after we're sure we'll submit work
    vkResetFences(m_device->device(), 1, &m_inFlightFences[m_currentFrame]);

    // Compute pass
    m_computePipeline->render(imageIndex,
                              m_currentFrame);  // Just record commands
    VkSubmitInfo computeSubmitInfo{};
    computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    computeSubmitInfo.waitSemaphoreCount = 1;
    computeSubmitInfo.pWaitSemaphores =
        &m_imageAvailableSemaphores[m_currentFrame];
    VkPipelineStageFlags computeWaitStage =
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    computeSubmitInfo.pWaitDstStageMask = &computeWaitStage;

    computeSubmitInfo.commandBufferCount = 1;
    computeSubmitInfo.pCommandBuffers =
        m_computePipeline->getCurrentCommandBuffer(m_currentFrame);
    computeSubmitInfo.signalSemaphoreCount = 1;
    computeSubmitInfo.pSignalSemaphores =
        &m_renderFinishedSemaphores[m_currentFrame];

    if (vkQueueSubmit(m_device->computeQueue(), 1, &computeSubmitInfo,
                      VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit compute command buffer!");
    }

    // // Graphics pass
    VkSubmitInfo graphicsSubmitInfo{};
    graphicsSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    graphicsSubmitInfo.waitSemaphoreCount = 1;
    graphicsSubmitInfo.pWaitSemaphores =
        &m_renderFinishedSemaphores[m_currentFrame];
    VkPipelineStageFlags graphicsWaitStage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    graphicsSubmitInfo.pWaitDstStageMask = &graphicsWaitStage;

    m_graphicsPipeline->render(imageIndex, m_currentFrame);  // Just record
    graphicsSubmitInfo.commandBufferCount = 1;
    graphicsSubmitInfo.pCommandBuffers =
        m_graphicsPipeline->getCurrentCommandBuffer(m_currentFrame);
    graphicsSubmitInfo.signalSemaphoreCount = 1;
    graphicsSubmitInfo.pSignalSemaphores =
        &m_renderFinishedSemaphores[m_currentFrame];

    if (vkQueueSubmit(m_device->graphicsQueue(), 1, &graphicsSubmitInfo,
                      m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit graphics command buffer!");
    }

    // Present
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapChain->getSwapChain();
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_device->presentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        m_framebufferResized) {
        m_framebufferResized = false;
        m_swapChain->recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_currentFrame = (m_currentFrame + 1) % config::MAX_FRAMES_IN_FLIGHT;
}
