#include "../includes/compute_pipeline.hpp"

#include <array>
#include <iostream>

#include "../includes/config.hpp"
#include "../includes/device_structures.hpp"
#include "../includes/scene.hpp"
#include "../includes/utils.hpp"

ComputePipeline::ComputePipeline(Device& device, SwapChain& swapChain,
                                 Scene& scene)
    : m_device(device), m_swapChain(swapChain), m_scene(scene) {
    createDescriptorSetLayout();
    createPipeline();
    createCommandPool();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
}

ComputePipeline::~ComputePipeline() {
    vkDestroyDescriptorSetLayout(m_device.device(), m_descriptorSetLayout,
                                 nullptr);
    // Add cleanup for uniform and sphere buffers
    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(m_device.device(), m_uniformBuffers[i], nullptr);
        vkFreeMemory(m_device.device(), m_uniformBuffersMemory[i], nullptr);
        vkDestroyBuffer(m_device.device(), m_sphereBuffers[i], nullptr);
        vkFreeMemory(m_device.device(), m_sphereBuffersMemory[i], nullptr);
    }

    vkDestroyPipeline(m_device.device(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    vkDestroyDescriptorPool(m_device.device(), m_descriptorPool, nullptr);

    vkDestroyCommandPool(m_device.device(), m_commandPool, nullptr);
}

void ComputePipeline::createPipeline() {
    // make shader
    auto computeShaderCode = readFile("../res/shaders/comp.spv");

    VkShaderModule computeShaderModule =
        createShaderModule(m_device.device(), computeShaderCode);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";
    //

    // make pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr,
                               &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }
    //

    // make pipeline
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.stage = computeShaderStageInfo;

    if (vkCreateComputePipelines(m_device.device(), VK_NULL_HANDLE, 1,
                                 &pipelineInfo, nullptr,
                                 &m_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    }

    // make input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // make rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // make multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    vkDestroyShaderModule(m_device.device(), computeShaderModule, nullptr);

    // make color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
}

void ComputePipeline::createCommandPool() {
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

void ComputePipeline::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[0].descriptorCount =
        static_cast<uint32_t>(config::MAX_FRAMES_IN_FLIGHT);
    // add sphere
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount =
        static_cast<uint32_t>(config::MAX_FRAMES_IN_FLIGHT);
    // add uniform
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[2].descriptorCount =
        static_cast<uint32_t>(config::MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(config::MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(m_device.device(), &poolInfo, nullptr,
                               &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void ComputePipeline::createCommandBuffers() {
    m_commandBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();
    if (vkAllocateCommandBuffers(m_device.device(), &allocInfo,
                                 m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate compute command buffers!");
    }
}

void ComputePipeline::createDescriptorSetLayout() {
    std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    // sphere
    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    // uniform
    layoutBindings[2].binding = 2;
    layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(m_device.device(), &layoutInfo, nullptr,
                                    &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error(
            "failed to create compute descriptor set layout!");
    }
}

void ComputePipeline::recordCommandBuffer(VkCommandBuffer commandBuffer,
                                          uint32_t currentFrame,
                                          uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error(
            "failed to begin recording compute command buffer!");
    }

    // Transition image to GENERAL layout before compute dispatch
    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageMemoryBarrier.image = m_swapChain.images()[imageIndex];
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;

    imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE_KHR;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &imageMemoryBarrier);

    // Bind and dispatch the compute shader
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                      m_pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            m_pipelineLayout, 0, 1,
                            &m_descriptorSets[currentFrame], 0, nullptr);

    vkCmdDispatch(commandBuffer,
                  static_cast<uint32_t>(m_swapChain.extent().width / 8),
                  static_cast<uint32_t>(m_swapChain.extent().height / 8), 1);

    VkImageMemoryBarrier bob{};
    // Transition the image back to PRESENT_SRC_KHR layout for presentation
    bob.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    bob.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    // bob.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    bob.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    bob.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bob.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bob.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    bob.dstAccessMask = VK_ACCESS_NONE_KHR;
    bob.image = m_swapChain.images()[imageIndex];
    bob.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bob.subresourceRange.baseMipLevel = 0;
    bob.subresourceRange.levelCount = 1;
    bob.subresourceRange.baseArrayLayer = 0;
    bob.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &bob);

    try {
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to record compute command buffer!");
        }
    } catch (std::runtime_error& e) {
        printf("Error: %s\n", e.what());
    }
}

// TODO: make generic
void ComputePipeline::createUniformBuffers() {
    VkDeviceSize sphereBufferSize = sizeof(Sphere) * m_scene.spheres().size();
    m_sphereBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
    m_sphereBuffersMemory.resize(config::MAX_FRAMES_IN_FLIGHT);
    m_sphereBuffersMapped.resize(config::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(m_device, sphereBufferSize,
                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_sphereBuffers[i], m_sphereBuffersMemory[i]);
        vkMapMemory(m_device.device(), m_sphereBuffersMemory[i], 0,
                    sphereBufferSize, 0, &m_sphereBuffersMapped[i]);
    }
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniformBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
    m_uniformBuffersMemory.resize(config::MAX_FRAMES_IN_FLIGHT);
    m_uniformBuffersMapped.resize(config::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(m_device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_uniformBuffers[i], m_uniformBuffersMemory[i]);
        vkMapMemory(m_device.device(), m_uniformBuffersMemory[i], 0, bufferSize,
                    0, &m_uniformBuffersMapped[i]);
    }
}

void ComputePipeline::updateScene(uint32_t currentImage) {
    memcpy(m_uniformBuffersMapped[currentImage], &m_scene.camera(),
           sizeof(m_scene.camera()));
    memcpy(m_sphereBuffersMapped[currentImage], m_scene.spheres().data(),
           m_scene.spheres().size() * sizeof(Sphere));
}

void ComputePipeline::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(config::MAX_FRAMES_IN_FLIGHT,
                                               m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(config::MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();
    m_descriptorSets.resize(config::MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_device.device(), &allocInfo,
                                 m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
        // Storage Image descriptor (binding = 0)
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = m_swapChain.imageViews()[i];
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageInfo.sampler = nullptr;

        // Storage Buffer descriptor for spheres (binding = 1)
        VkDescriptorBufferInfo sphereBufferInfo{};
        sphereBufferInfo.buffer = m_sphereBuffers[i];
        sphereBufferInfo.offset = 0;
        sphereBufferInfo.range = sizeof(Sphere) * m_scene.spheres().size();

        // Uniform Buffer descriptor (binding = 2)
        VkDescriptorBufferInfo uniformBufferInfo{};
        uniformBufferInfo.buffer = m_uniformBuffers[i];
        uniformBufferInfo.offset = 0;
        uniformBufferInfo.range = sizeof(UniformBufferObject);

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        // Storage Image (binding = 0)
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &imageInfo;

        // Sphere Storage Buffer (binding = 1)
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &sphereBufferInfo;

        // Uniform Buffer (binding = 2)
        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = m_descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &uniformBufferInfo;

        try {
            vkUpdateDescriptorSets(
                m_device.device(),
                static_cast<uint32_t>(descriptorWrites.size()),
                descriptorWrites.data(), 0, nullptr);
        } catch (const std::exception& e) {
            std::cerr << "Error updating descriptor sets: " << e.what()
                      << std::endl;
            throw std::runtime_error("failed to update descriptor sets!");
        }
    }
}

void ComputePipeline::updateDescriptorSets(uint32_t imageIndex,
                                           uint32_t currentFrame) {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = m_swapChain.imageViews()[imageIndex];
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageInfo.sampler = nullptr;
    int i = currentFrame;
    // Storage Buffer descriptor for spheres (binding = 1)
    VkDescriptorBufferInfo sphereBufferInfo{};
    sphereBufferInfo.buffer = m_sphereBuffers[i];
    sphereBufferInfo.offset = 0;
    sphereBufferInfo.range = sizeof(Sphere) * m_scene.camera().sphereCount;

    // Uniform Buffer descriptor (binding = 2)
    VkDescriptorBufferInfo uniformBufferInfo{};
    uniformBufferInfo.buffer = m_uniformBuffers[i];
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = sizeof(UniformBufferObject);

    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

    // Storage Image (binding = 0)
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_descriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pImageInfo = &imageInfo;

    // Sphere Storage Buffer (binding = 1)
    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_descriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &sphereBufferInfo;

    // Uniform Buffer (binding = 2)
    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = m_descriptorSets[i];
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &uniformBufferInfo;
    vkUpdateDescriptorSets(m_device.device(),
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
}

void ComputePipeline::render(uint32_t imageIndex, uint32_t currentFrame) {
    // vkWaitForFences(m_device.device(), 1, &m_inFlightFences[m_currentFrame],
    //                 VK_TRUE, UINT64_MAX);

    // uint32_t imageIndex;
    // VkResult result = vkAcquireNextImageKHR(
    //     m_device.device(), m_swapChain.getSwapChain(), UINT64_MAX,
    //     m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE,
    //     &imageIndex);

    // if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    //     m_swapChain.recreateSwapChain();
    //     return;
    // } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    //     throw std::runtime_error("failed to acquire swap chain image!");
    // }

    // vkResetFences(m_device.device(), 1, &m_inFlightFences[m_currentFrame]);

    updateScene(currentFrame);
    updateDescriptorSets(imageIndex, currentFrame);
    vkResetCommandBuffer(m_commandBuffers[currentFrame], 0);
    recordCommandBuffer(m_commandBuffers[currentFrame], currentFrame,
                        imageIndex);

    // VkPipelineStageFlags waitStages[] = {
    //     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    // VkSubmitInfo computeSubmitInfo{};
    // computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // computeSubmitInfo.waitSemaphoreCount = 1;
    // computeSubmitInfo.pWaitSemaphores = &imageAvailableSemaphore;
    // computeSubmitInfo.pWaitDstStageMask = waitStages;
    // computeSubmitInfo.commandBufferCount = 1;
    // computeSubmitInfo.pCommandBuffers = &m_commandBuffers[currentFrame];
    // computeSubmitInfo.signalSemaphoreCount = 1;
    // computeSubmitInfo.pSignalSemaphores = &renderFinishedSemaphore;

    // if (vkQueueSubmit(m_device.computeQueue(), 1, &computeSubmitInfo,
    //                   inFlightFence) != VK_SUCCESS) {
    //     throw std::runtime_error("failed to submit compute command buffer!");
    // }

    // // Present
    // VkPresentInfoKHR presentInfo{};
    // presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // presentInfo.waitSemaphoreCount = 1;
    // presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
    // presentInfo.swapchainCount = 1;
    // presentInfo.pSwapchains = &m_swapChain.getSwapChain();
    // presentInfo.pImageIndices = &imageIndex;

    // VkResult result;
    // try {
    //     result = vkQueuePresentKHR(m_device.presentQueue(), &presentInfo);
    // } catch (const std::exception& e) {
    //     printf("Error in vkQueuePresentKHR: %s\n", e.what());
    // }

    // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
    //     m_framebufferResized) {
    //     m_framebufferResized = false;
    //     m_swapChain.recreateSwapChain();
    // } else if (result != VK_SUCCESS) {
    //     throw std::runtime_error("failed to present swap chain image!");
    // }
    // m_currentFrame = (m_currentFrame + 1) % config::MAX_FRAMES_IN_FLIGHT;
}
