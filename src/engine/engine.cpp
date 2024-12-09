#include "engine.hpp"

#include "includes/compute_pipeline.hpp"
#include "includes/config.hpp"
#include "includes/device_structures.hpp"
#include "includes/instance.hpp"
#include "includes/swap_chain.hpp"
#include "includes/utils.hpp"

Engine::Engine(uint32_t width, uint32_t height, GLFWwindow* window)
    : m_window(window) {
    initVulkan();
}

void Engine::initVulkan() {
    m_instance = std::make_unique<Instance>(m_window);
    m_device = std::make_unique<Device>(m_instance->getInstance(),
                                        m_instance->getSurface());
    m_swapChain = std::make_unique<SwapChain>(*m_device, m_window,
                                              m_instance->getSurface());
    m_computePipeline =
        std::make_unique<ComputePipeline>(*m_device, *m_swapChain);
    // createComputeDescriptorSetLayout();
    // createComputePipeline();
    // createCommandPool();
    // for uniforms
    // createUniformBuffers();
    // createDescriptorPool();
    // createDescriptorSets();
    // createComputeDescriptorSets();
    // createComputeCommandBuffers();
    // createSyncObjects();
}

void Engine::cleanup() {
    vkDeviceWaitIdle(m_device->device());
    m_swapChain.reset();
    m_computePipeline.reset();
    // vkDestroyDescriptorSetLayout(m_device->device(),
    //                              m_computeDescriptorSetLayout, nullptr);

    // Add cleanup for uniform and sphere buffers
    // for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    //     vkDestroyBuffer(m_device->device(), m_uniformBuffers[i], nullptr);
    //     vkFreeMemory(m_device->device(), m_uniformBuffersMemory[i], nullptr);
    //     vkDestroyBuffer(m_device->device(), m_sphereBuffers[i], nullptr);
    //     vkFreeMemory(m_device->device(), m_sphereBuffersMemory[i], nullptr);
    // }

    // vkDestroyPipeline(m_device->device(), m_computePipeline, nullptr);
    // vkDestroyPipelineLayout(m_device->device(), m_computePipelineLayout,
    //                         nullptr);

    // vkDestroyDescriptorPool(m_device->device(), m_descriptorPool, nullptr);

    // for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    //     vkDestroySemaphore(m_device->device(), m_renderFinishedSemaphores[i],
    //                        nullptr);
    //     vkDestroySemaphore(m_device->device(), m_imageAvailableSemaphores[i],
    //                        nullptr);
    //     vkDestroyFence(m_device->device(), m_inFlightFences[i], nullptr);
    // }

    // vkDestroyCommandPool(m_device->device(), m_commandPool, nullptr);
    m_device.reset();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

// void Engine::createRenderPass() {
//     VkAttachmentDescription colorAttachment{};
//     colorAttachment.format = m_swapChain->imageFormat();
//     colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//     colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//     colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//     colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//     colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//     colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//     colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

//     VkAttachmentReference colorAttachmentRef{};
//     colorAttachmentRef.attachment = 0;
//     colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

//     VkSubpassDescription subpass{};
//     subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//     subpass.colorAttachmentCount = 1;
//     subpass.pColorAttachments = &colorAttachmentRef;

//     VkSubpassDependency dependency{};
//     dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//     dependency.dstSubpass = 0;
//     dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//     dependency.srcAccessMask = 0;
//     dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//     dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

//     VkRenderPassCreateInfo renderPassInfo{};
//     renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//     renderPassInfo.attachmentCount = 1;
//     renderPassInfo.pAttachments = &colorAttachment;
//     renderPassInfo.subpassCount = 1;
//     renderPassInfo.pSubpasses = &subpass;
//     renderPassInfo.dependencyCount = 1;
//     renderPassInfo.pDependencies = &dependency;

//     if (vkCreateRenderPass(m_device->device(), &renderPassInfo, nullptr,
//                            &m_renderPass) != VK_SUCCESS) {
//         throw std::runtime_error("failed to create render pass!");
//     }
// }

// for uniforms

// void Engine::createComputeDescriptorSets() {
// std::vector<VkDescriptorSetLayout> layouts(config::MAX_FRAMES_IN_FLIGHT,
//                                            m_computeDescriptorSetLayout);
// printf("Creating compute descriptor sets\n");
// VkDescriptorSetAllocateInfo allocInfo{};
// allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
// allocInfo.descriptorPool = m_descriptorPool;
// allocInfo.descriptorSetCount =
//     static_cast<uint32_t>(config::MAX_FRAMES_IN_FLIGHT);
// allocInfo.pSetLayouts = layouts.data();

// m_computeDescriptorSets.resize(config::MAX_FRAMES_IN_FLIGHT);
// if (vkAllocateDescriptorSets(m_device->device(), &allocInfo,
//                              m_computeDescriptorSets.data()) !=
//     VK_SUCCESS) {
//     throw std::runtime_error("failed to allocate descriptor sets!");
// }

// for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
//     VkDescriptorImageInfo imageInfo{};
//     imageInfo.imageView = m_swapChain->imageViews()[i];
//     imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
//     imageInfo.sampler = nullptr;

//     VkWriteDescriptorSet descriptorWrite{};
//     descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//     descriptorWrite.dstSet = m_computeDescriptorSets[i];
//     descriptorWrite.dstBinding = 0;
//     descriptorWrite.dstArrayElement = 0;
//     descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
//     descriptorWrite.descriptorCount = 1;
//     descriptorWrite.pImageInfo = &imageInfo;

//     vkUpdateDescriptorSets(m_device->device(), 1, &descriptorWrite, 0,
//                            nullptr);
// }
// }
