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
    createAccumulationImage();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
}

ComputePipeline::~ComputePipeline() {
    vkDestroyDescriptorSetLayout(m_device.device(), m_descriptorSetLayout,
                                 nullptr);
    // Cleanup accumulation resources
    if (m_accumulationImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device.device(), m_accumulationImageView, nullptr);
    }
    if (m_accumulationImage != VK_NULL_HANDLE) {
        vkDestroyImage(m_device.device(), m_accumulationImage, nullptr);
    }
    if (m_accumulationImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device.device(), m_accumulationImageMemory, nullptr);
    }
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

void ComputePipeline::windowResized() {
    vkDeviceWaitIdle(m_device.device());
    if (m_accumulationImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device.device(), m_accumulationImageView, nullptr);
    }
    if (m_accumulationImage != VK_NULL_HANDLE) {
        vkDestroyImage(m_device.device(), m_accumulationImage, nullptr);
    }
    if (m_accumulationImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device.device(), m_accumulationImageMemory, nullptr);
    }
    createAccumulationImage();
    m_scene.resetFrameCount();
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

void ComputePipeline::createAccumulationImage() {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format =
        VK_FORMAT_R32G32B32A32_SFLOAT;  // Use float for HDR accumulation
    imageInfo.extent.width = m_swapChain.extent().width;
    imageInfo.extent.height = m_swapChain.extent().height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.usage =
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (vkCreateImage(m_device.device(), &imageInfo, nullptr,
                      &m_accumulationImage) != VK_SUCCESS) {
        throw std::runtime_error("failed to create accumulation image!");
    }

    // Allocate memory for the image
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device.device(), m_accumulationImage,
                                 &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(m_device, memRequirements.memoryTypeBits,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(m_device.device(), &allocInfo, nullptr,
                         &m_accumulationImageMemory) != VK_SUCCESS) {
        throw std::runtime_error(
            "failed to allocate accumulation image memory!");
    }

    vkBindImageMemory(m_device.device(), m_accumulationImage,
                      m_accumulationImageMemory, 0);

    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_accumulationImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device.device(), &viewInfo, nullptr,
                          &m_accumulationImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create accumulation image view!");
    }
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_accumulationImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (VK_IMAGE_LAYOUT_UNDEFINED == VK_IMAGE_LAYOUT_UNDEFINED &&
        VK_IMAGE_LAYOUT_GENERAL == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);

    endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer ComputePipeline::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device.device(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void ComputePipeline::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_device.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device.graphicsQueue());

    vkFreeCommandBuffers(m_device.device(), m_commandPool, 1, &commandBuffer);
}

void ComputePipeline::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 3> poolSizes{};

    // Storage Images (for both color and accumulation)
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(
        config::MAX_FRAMES_IN_FLIGHT);  // Two storage images per frame

    // Storage Buffer (for spheres)
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount =
        static_cast<uint32_t>(config::MAX_FRAMES_IN_FLIGHT);

    // Uniform Buffer (for scene data)
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[2].descriptorCount =
        static_cast<uint32_t>(config::MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(config::MAX_FRAMES_IN_FLIGHT);
    poolInfo.flags = 0;

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
    std::array<VkDescriptorSetLayoutBinding, 4> layoutBindings{};

    // Binding 0: Output image (colorBuffer)
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[0].pImmutableSamplers = nullptr;

    // Binding 1: Accumulation image
    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[1].pImmutableSamplers = nullptr;

    // Binding 2: Sphere buffer (SphereData)
    layoutBindings[2].binding = 2;
    layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[2].pImmutableSamplers = nullptr;

    // Binding 3: Uniform buffer (SceneData)
    layoutBindings[3].binding = 3;
    layoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[3].descriptorCount = 1;
    layoutBindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[3].pImmutableSamplers = nullptr;

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
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // 1. Transition swapchain image from PRESENT_SRC to GENERAL
    VkImageMemoryBarrier presentToCompute{};
    presentToCompute.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    presentToCompute.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    presentToCompute.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    presentToCompute.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    presentToCompute.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    presentToCompute.image = m_swapChain.images()[imageIndex];
    presentToCompute.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    presentToCompute.subresourceRange.baseMipLevel = 0;
    presentToCompute.subresourceRange.levelCount = 1;
    presentToCompute.subresourceRange.baseArrayLayer = 0;
    presentToCompute.subresourceRange.layerCount = 1;
    presentToCompute.srcAccessMask = 0;
    presentToCompute.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &presentToCompute);

    // Bind pipeline and descriptor set
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                      m_pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            m_pipelineLayout, 0, 1,
                            &m_descriptorSets[currentFrame], 0, nullptr);

    // Dispatch compute shader
    vkCmdDispatch(commandBuffer,
                  static_cast<uint32_t>(m_swapChain.extent().width / 8),
                  static_cast<uint32_t>(m_swapChain.extent().height / 8), 1);

    // 2. Transition swapchain image to COLOR_ATTACHMENT_OPTIMAL for UI
    // rendering
    VkImageMemoryBarrier computeToGraphics{};
    computeToGraphics.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    computeToGraphics.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    computeToGraphics.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    computeToGraphics.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    computeToGraphics.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    computeToGraphics.image = m_swapChain.images()[imageIndex];
    computeToGraphics.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    computeToGraphics.subresourceRange.baseMipLevel = 0;
    computeToGraphics.subresourceRange.levelCount = 1;
    computeToGraphics.subresourceRange.baseArrayLayer = 0;
    computeToGraphics.subresourceRange.layerCount = 1;
    computeToGraphics.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    computeToGraphics.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &computeToGraphics);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
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

        // Accumulation buffer (binding = 1)
        VkDescriptorImageInfo accumImageInfo{};
        accumImageInfo.imageView = m_accumulationImageView;
        accumImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

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

        std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

        // Storage Image (binding = 0)
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &imageInfo;

        // Accumulation buffer (binding = 1)

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &accumImageInfo;

        // Sphere Storage Buffer (binding = 1)
        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = m_descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &sphereBufferInfo;

        // Uniform Buffer (binding = 2)
        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = m_descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pBufferInfo = &uniformBufferInfo;

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
    // Image descriptors for the color buffer
    VkDescriptorImageInfo colorImageInfo{};
    colorImageInfo.imageView = m_swapChain.imageViews()[imageIndex];
    colorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    colorImageInfo.sampler = nullptr;

    // Image descriptors for the accumulation buffer
    VkDescriptorImageInfo accumImageInfo{};
    accumImageInfo.imageView = m_accumulationImageView;
    accumImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    // Buffer descriptor for the sphere data
    VkDescriptorBufferInfo sphereBufferInfo{};
    sphereBufferInfo.buffer = m_sphereBuffers[currentFrame];
    sphereBufferInfo.offset = 0;
    sphereBufferInfo.range = sizeof(Sphere) * m_scene.spheres().size();

    // Buffer descriptor for the uniform data
    VkDescriptorBufferInfo uniformBufferInfo{};
    uniformBufferInfo.buffer = m_uniformBuffers[currentFrame];
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = sizeof(UniformBufferObject);

    std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

    // Binding 0: Color buffer
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_descriptorSets[currentFrame];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pImageInfo = &colorImageInfo;

    // Binding 1: Accumulation buffer
    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_descriptorSets[currentFrame];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &accumImageInfo;

    // Binding 2: Sphere buffer
    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = m_descriptorSets[currentFrame];
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &sphereBufferInfo;

    // Binding 3: Uniform buffer
    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[3].dstSet = m_descriptorSets[currentFrame];
    descriptorWrites[3].dstBinding = 3;
    descriptorWrites[3].dstArrayElement = 0;
    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[3].descriptorCount = 1;
    descriptorWrites[3].pBufferInfo = &uniformBufferInfo;

    vkUpdateDescriptorSets(m_device.device(),
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
}

void ComputePipeline::render(uint32_t imageIndex, uint32_t currentFrame) {
    updateScene(currentFrame);
    updateDescriptorSets(imageIndex, currentFrame);
    vkResetCommandBuffer(m_commandBuffers[currentFrame], 0);
    recordCommandBuffer(m_commandBuffers[currentFrame], currentFrame,
                        imageIndex);
}
