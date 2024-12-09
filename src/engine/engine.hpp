
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include "includes/device.hpp"
#include "includes/swap_chain.hpp"

struct UniformBufferObject {
    alignas(16) glm::vec3 camera_forward;
    alignas(16) glm::vec3 camera_right;
    alignas(16) glm::vec3 camera_up;
    alignas(16) glm::vec3 camera_position;
    alignas(4) int sphereCount;
};

struct Sphere {
    alignas(16) glm::vec3 center;  // Aligned to 16 bytes
    alignas(4) float radius;       // Aligned to 4 bytes
    alignas(16) glm::vec3 color;   // Aligned to 16 bytes
    alignas(4) float padding;      // Add padding to ensure 16-byte alignment
};

inline VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

inline void DestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2>
    getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2>
            attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

class Engine {
   public:
    Engine(uint32_t width, uint32_t height, GLFWwindow* window);
    ~Engine() { cleanup(); };
    void render();
    void setFramebufferResized(bool resized) { m_framebufferResized = resized; }

   private:
    void initVulkan();
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void createRenderPass();
    void createComputeDescriptorSetLayout();
    void createComputePipeline();
    void createCommandPool();
    void createComputeDescriptorSets();
    void createSyncObjects();
    void createComputeCommandBuffers();
    void createUniformBuffers();
    void createDescriptorSets();
    void createDescriptorPool();

    void updateUniformBuffer(uint32_t currentImage);

    void recordComputeCommandBuffer(VkCommandBuffer commandBuffer,
                                    uint32_t imageIndex);
    void cleanup();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties, VkBuffer& buffer,
                      VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    uint32_t findMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);
    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage
                  << std::endl;

        return VK_FALSE;
    }

   private:
    // window
    GLFWwindow* m_window;

    // instance
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    VkSurfaceKHR m_surface;

    std::unique_ptr<Device> m_device;
    std::unique_ptr<SwapChain> m_swapChain;

    // render pass
    VkRenderPass m_renderPass;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;

    // compute
    VkDescriptorSetLayout m_computeDescriptorSetLayout;
    VkPipelineLayout m_computePipelineLayout;
    VkPipeline m_computePipeline;

    VkCommandPool m_commandPool;

    // sphere and scene buffer
    std::vector<VkBuffer> m_sphereBuffers;
    std::vector<VkDeviceMemory> m_sphereBuffersMemory;
    std::vector<void*> m_sphereBuffersMapped;

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;

    // command
    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkCommandBuffer> m_computeCommandBuffers;

    // descriptor
    VkDescriptorPool m_descriptorPool;
    // std::vector<VkDescriptorSet> m_computeDescriptorSets;

    // sync
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    // for uniforms
    // VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;
    // ??
    // VkDescriptorSetLayout m_descriptorSetLayout;
    uint32_t m_currentFrame = 0;
    bool m_framebufferResized = false;

    // TODO: bouger
    std::vector<Sphere> m_spheres;
    UniformBufferObject m_camera;
};
