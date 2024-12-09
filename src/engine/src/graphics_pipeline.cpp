#include "../includes/graphics_pipeline.hpp"

#include <stdexcept>

#include "../includes/config.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
GraphicsPipeline::GraphicsPipeline(Device& device, SwapChain& swapChain,
                                   Instance& instance, GLFWwindow* window)
    : m_device(device),
      m_swapChain(swapChain),
      m_window(window),
      m_instance(instance) {
    createCommandPool();
    createCommandBuffers();
    initImGui();
}

void GraphicsPipeline::initImGui() {
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = std::size(pool_sizes);
    poolInfo.pPoolSizes = pool_sizes;

    if (vkCreateDescriptorPool(m_device.device(), &poolInfo, nullptr,
                               &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool for ImGui!");
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(m_window, true);

    // Initialize ImGui for Vulkan
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_instance.getInstance();
    initInfo.PhysicalDevice = m_device.physicalDevice();
    initInfo.Device = m_device.device();
    initInfo.Queue = m_device.graphicsQueue();
    initInfo.DescriptorPool = m_descriptorPool;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = m_swapChain.imageCount();
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.RenderPass = m_swapChain.getRenderPass();
    initInfo.UseDynamicRendering = false;

    if (!ImGui_ImplVulkan_Init(&initInfo)) {
        throw std::runtime_error("Failed to initialize ImGui with Vulkan!");
    }

    // Upload fonts
    ImGui_ImplVulkan_CreateFontsTexture();
}

void GraphicsPipeline::render(uint32_t imageIndex, uint32_t currentFrame) {
    // vkWaitForFences(m_device.device(), 1, &m_inFlightFences[m_currentFrame],
    //                 VK_TRUE, UINT64_MAX);

    // uint32_t imageIndex;
    // VkResult result = vkAcquireNextImageKHR(
    //     m_device.device(), m_swapChain.getSwapChain(), UINT64_MAX,
    //     m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE,
    //     &imageIndex);

    // if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    //     // Handle swapchain recreation
    //     return;
    // }

    // vkResetFences(m_device.device(), 1, &m_inFlightFences[m_currentFrame]);
    // Update ImGui UI
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Add your ImGui widgets here
    ImGui::Begin("Hello, ImGui!");
    ImGui::Text("This is an ImGui window");
    ImGui::End();

    ImGui::Render();

    // Record command buffer
    vkResetCommandBuffer(m_commandBuffers[currentFrame], 0);
    recordCommandBuffer(m_commandBuffers[currentFrame], imageIndex);

    // Submit command buffer
    // VkSubmitInfo submitInfo{};
    // submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    // VkPipelineStageFlags waitStages[] = {
    //     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    // submitInfo.waitSemaphoreCount = 1;
    // submitInfo.pWaitSemaphores = waitSemaphores;
    // submitInfo.pWaitDstStageMask = waitStages;
    // submitInfo.commandBufferCount = 1;
    // submitInfo.pCommandBuffers = &m_commandBuffers[currentFrame];

    // VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    // submitInfo.signalSemaphoreCount = 1;
    // submitInfo.pSignalSemaphores = signalSemaphores;

    // if (vkQueueSubmit(m_device.graphicsQueue(), 1, &submitInfo,
    //                   inFlightFence) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to submit draw command buffer!");
    // }

    // Present
    // VkPresentInfoKHR presentInfo{};
    // presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // presentInfo.waitSemaphoreCount = 1;
    // presentInfo.pWaitSemaphores = signalSemaphores;

    // VkSwapchainKHR swapChains[] = {m_swapChain.getSwapChain()};
    // presentInfo.swapchainCount = 1;
    // presentInfo.pSwapchains = swapChains;
    // presentInfo.pImageIndices = &imageIndex;
    // VkResult result;

    // result = vkQueuePresentKHR(m_device.presentQueue(), &presentInfo);

    // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
    //     m_framebufferResized) {
    //     m_framebufferResized = false;
    // Handle swapchain recreation
    // }

    // m_currentFrame = (m_currentFrame + 1) % config::MAX_FRAMES_IN_FLIGHT;
}

GraphicsPipeline::~GraphicsPipeline() {
    vkDeviceWaitIdle(m_device.device());

    // Cleanup ImGui
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device.device(), m_commandPool, nullptr);
    }
    if (m_descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device.device(), m_descriptorPool, nullptr);
    }
}

void GraphicsPipeline::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices =
        m_device.findQueueFamilies(m_device.physicalDevice());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex =
        queueFamilyIndices.graphicsAndComputeFamily.value();

    if (vkCreateCommandPool(m_device.device(), &poolInfo, nullptr,
                            &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void GraphicsPipeline::createCommandBuffers() {
    m_commandBuffers.resize(m_swapChain.imageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount =
        static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_device.device(), &allocInfo,
                                 m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void GraphicsPipeline::recordCommandBuffer(VkCommandBuffer commandBuffer,
                                           uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_swapChain.getRenderPass();
    renderPassInfo.framebuffer = m_swapChain.getFramebuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapChain.extent();

    renderPassInfo.pClearValues = nullptr;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    // Record ImGui Draw Data
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}