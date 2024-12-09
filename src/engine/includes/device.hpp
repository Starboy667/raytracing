#pragma once
#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

struct QueueFamilyIndices;
struct SwapChainSupportDetails;

class Device {
   public:
    Device(const VkInstance& instance, const VkSurfaceKHR& surface);
    ~Device();

    // Delete copy constructors
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    // Getters
    VkDevice device() const { return m_device; }
    VkPhysicalDevice physicalDevice() const { return m_physicalDevice; }
    VkQueue graphicsQueue() const { return m_graphicsQueue; }
    VkQueue computeQueue() const { return m_computeQueue; }
    VkQueue presentQueue() const { return m_presentQueue; }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

   private:
    void pickPhysicalDevice();
    void createLogicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

   private:
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device;

    // Queue handles (these are tied to the logical device)
    VkQueue m_graphicsQueue;
    VkQueue m_computeQueue;
    VkQueue m_presentQueue;

    VkInstance m_instance;
    VkSurfaceKHR m_surface;
};