#include "../includes/graphics_pipeline.hpp"

#include <stdexcept>

#include "../includes/config.hpp"

GraphicsPipeline::GraphicsPipeline(Device& device, SwapChain& swapChain,
                                   Instance& instance, GLFWwindow* window,
                                   Scene& scene)
    : m_device(device),
      m_swapChain(swapChain),
      m_window(window),
      m_instance(instance),
      m_scene(scene) {
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
    ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

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
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (config::show_demo_window == true)
        ImGui::ShowDemoWindow(&config::show_demo_window);
    ImGui::Begin("Raytracing");
    ImGui::Text("Settings");
    // TODO: clean
    float old_camera_position_x = m_scene.m_camera.camera_position.x;
    float old_camera_position_y = m_scene.m_camera.camera_position.y;
    float old_camera_position_z = m_scene.m_camera.camera_position.z;
    float gap = 100.0f;
    ImGui::Text("Accumulated frame count: %d", m_scene.m_camera.frameCount);
    ImGui::Separator();
    if (ImGui::Button("Reset frame count")) {
        m_scene.m_camera.frameCount = 0;
    }
    ImGui::SliderFloat("camera.x", &m_scene.m_camera.camera_position.x, -gap,
                       gap, "%.3f");
    ImGui::SliderFloat("camera.y", &m_scene.m_camera.camera_position.y, -gap,
                       gap, "%.3f");
    ImGui::SliderFloat("camera.z", &m_scene.m_camera.camera_position.z, -gap,
                       gap, "%.3f");
    if (old_camera_position_x != m_scene.m_camera.camera_position.x ||
        old_camera_position_y != m_scene.m_camera.camera_position.y ||
        old_camera_position_z != m_scene.m_camera.camera_position.z) {
        m_scene.m_camera.frameCount = 0;
    }
    ImGui::Separator();
    for (int i = 0; i < m_scene.spheres().size(); i++) {
        ImGui::PushID(i);
        char label[32];
        sprintf_s(label, "Object number %d", i);
        if (ImGui::CollapsingHeader(label)) {
            // if (ImGui::CollapsingHeader("Object number %d", i)) {
            ImGui::SliderFloat("sphere.x", &m_scene.m_spheres[i].center.x,
                               -10.0f, 10.0f, "%.3f");
            ImGui::SliderFloat("sphere.y", &m_scene.m_spheres[i].center.y,
                               -10.0f, 10.0f, "%.3f");
            ImGui::SliderFloat("sphere.z", &m_scene.m_spheres[i].center.z,
                               -10.0f, 10.0f, "%.3f");
        }
        ImGui::PopID();
    }
    ImGui::Separator();
    ImGui::End();

    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
    vkResetCommandBuffer(m_commandBuffers[currentFrame], 0);
    recordCommandBuffer(m_commandBuffers[currentFrame], imageIndex);
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