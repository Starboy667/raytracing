#include "engine.hpp"

#include "includes/config.hpp"
#include "includes/device_structures.hpp"
#include "includes/swap_chain.hpp"
#include "includes/utils.hpp"

Engine::Engine(uint32_t width, uint32_t height, GLFWwindow* window)
    : m_window(window) {
    m_camera.camera_position =
        glm::vec3(0.0f, 0.0f, -10.0f);                      // Move camera back
    m_camera.camera_forward = glm::vec3(0.0f, 0.0f, 1.0f);  // Looking along +Z
    m_camera.camera_right = glm::vec3(1.0f, 0.0f, 0.0f);    // Right along +X
    m_camera.camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_camera.sphereCount = 10;

    m_spheres.reserve(m_camera.sphereCount);
    for (int i = 0; i < m_camera.sphereCount; i++) {
        float x = random_float(-5.0f, 5.0f);
        float y = random_float(-5.0f, 5.0f);
        float z = random_float(-5.0f, 5.0f);
        float r = random_float();
        float g = random_float();
        float b = random_float();
        float radius = random_float(0.5f, 3.0f);

        Sphere sphere{};
        sphere.center = glm::vec3(x, y, z);
        sphere.radius = radius;
        sphere.color = glm::vec3(r, g, b);
        m_spheres.push_back(sphere);
    }
    initVulkan();
}

void Engine::initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    // pickPhysicalDevice();
    m_device = std::make_unique<Device>(m_instance, m_surface);
    // createLogicalDevice();
    // createSwapChain();
    // createImageViews();
    m_swapChain = std::make_unique<SwapChain>(*m_device, m_window, m_surface);
    createComputeDescriptorSetLayout();
    createComputePipeline();
    createCommandPool();
    // for uniforms
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    // createComputeDescriptorSets();
    createComputeCommandBuffers();
    createSyncObjects();
}

void Engine::cleanup() {
    vkDeviceWaitIdle(m_device->device());
    // m_swapChain->cleanupSwapChain();
    m_swapChain.reset();
    vkDestroyDescriptorSetLayout(m_device->device(),
                                 m_computeDescriptorSetLayout, nullptr);

    // Add cleanup for uniform and sphere buffers
    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(m_device->device(), m_uniformBuffers[i], nullptr);
        vkFreeMemory(m_device->device(), m_uniformBuffersMemory[i], nullptr);
        vkDestroyBuffer(m_device->device(), m_sphereBuffers[i], nullptr);
        vkFreeMemory(m_device->device(), m_sphereBuffersMemory[i], nullptr);
    }

    vkDestroyPipeline(m_device->device(), m_computePipeline, nullptr);
    vkDestroyPipelineLayout(m_device->device(), m_computePipelineLayout,
                            nullptr);

    vkDestroyDescriptorPool(m_device->device(), m_descriptorPool, nullptr);

    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_device->device(), m_renderFinishedSemaphores[i],
                           nullptr);
        vkDestroySemaphore(m_device->device(), m_imageAvailableSemaphores[i],
                           nullptr);
        vkDestroyFence(m_device->device(), m_inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(m_device->device(), m_commandPool, nullptr);
    m_device.reset();
    // vkDestroyDevice(m_device->device(), nullptr);

    if (config::enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Engine::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapChain->imageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_device->device(), &renderPassInfo, nullptr,
                           &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Engine::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices =
        m_device->findQueueFamilies(m_device->physicalDevice());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex =
        queueFamilyIndices.graphicsAndComputeFamily.value();

    if (vkCreateCommandPool(m_device->device(), &poolInfo, nullptr,
                            &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void Engine::createSyncObjects() {
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
                              &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device->device(), &semaphoreInfo, nullptr,
                              &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_device->device(), &fenceInfo, nullptr,
                          &m_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to create graphics synchronization objects for a "
                "frame!");
        }
    }
}

// for uniforms
void Engine::createDescriptorPool() {
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

    if (vkCreateDescriptorPool(m_device->device(), &poolInfo, nullptr,
                               &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void Engine::createComputeDescriptorSetLayout() {
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

    if (vkCreateDescriptorSetLayout(m_device->device(), &layoutInfo, nullptr,
                                    &m_computeDescriptorSetLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error(
            "failed to create compute descriptor set layout!");
    }
}

void Engine::createComputeDescriptorSets() {
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
}

void Engine::createComputeCommandBuffers() {
    m_computeCommandBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_computeCommandBuffers.size();
    if (vkAllocateCommandBuffers(m_device->device(), &allocInfo,
                                 m_computeCommandBuffers.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to allocate compute command buffers!");
    }
}

void Engine::createComputePipeline() {
    // make shader
    auto computeShaderCode = readFile("../res/shaders/comp.spv");

    VkShaderModule computeShaderModule = createShaderModule(computeShaderCode);

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
    pipelineLayoutInfo.pSetLayouts = &m_computeDescriptorSetLayout;

    if (vkCreatePipelineLayout(m_device->device(), &pipelineLayoutInfo, nullptr,
                               &m_computePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }
    //

    // make pipeline
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = m_computePipelineLayout;
    pipelineInfo.stage = computeShaderStageInfo;

    if (vkCreateComputePipelines(m_device->device(), VK_NULL_HANDLE, 1,
                                 &pipelineInfo, nullptr,
                                 &m_computePipeline) != VK_SUCCESS) {
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
    vkDestroyShaderModule(m_device->device(), computeShaderModule, nullptr);

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

void Engine::recordComputeCommandBuffer(VkCommandBuffer commandBuffer,
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
    imageMemoryBarrier.image = m_swapChain->images()[imageIndex];
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
                      m_computePipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            m_computePipelineLayout, 0, 1,
                            &m_descriptorSets[m_currentFrame], 0, nullptr);

    vkCmdDispatch(commandBuffer,
                  static_cast<uint32_t>(m_swapChain->extent().width / 8),
                  static_cast<uint32_t>(m_swapChain->extent().height / 8), 1);

    VkImageMemoryBarrier bob{};
    // Transition the image back to PRESENT_SRC_KHR layout for presentation
    bob.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    bob.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    bob.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    bob.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bob.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bob.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    bob.dstAccessMask = VK_ACCESS_NONE_KHR;
    bob.image = m_swapChain->images()[imageIndex];
    bob.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bob.subresourceRange.baseMipLevel = 0;
    bob.subresourceRange.levelCount = 1;
    bob.subresourceRange.baseArrayLayer = 0;
    bob.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &bob);

    try {
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to record compute command buffer!");
        }
    } catch (std::runtime_error& e) {
        printf("Error: %s\n", e.what());
    }
}
