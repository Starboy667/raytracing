#pragma once

#include "device.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
class SwapChain {
   public:
    SwapChain(Device& device, GLFWwindow* window, VkSurfaceKHR surface);
    ~SwapChain();
    VkFormat imageFormat() const { return m_imageFormat; }
    const std::vector<VkImageView>& imageViews() const { return m_imageViews; }
    const std::vector<VkImage>& images() const { return m_images; }
    const VkExtent2D& extent() const { return m_extent; }
    const VkSwapchainKHR& getSwapChain() const { return m_swapChain; }
    const VkRenderPass& getRenderPass() const { return m_renderPass; }
    const std::vector<VkFramebuffer>& getFramebuffers() const {
        return m_framebuffers;
    }

    uint32_t imageCount() const { return m_imageCount; }
    void recreateSwapChain();

   private:
    void createSwapChain();
    void createImageViews();
    void createFramebuffers();
    void createRenderPass();
    void cleanupSwapChain();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

   private:
    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    VkFormat m_imageFormat;
    VkExtent2D m_extent;
    Device& m_device;
    VkSurfaceKHR m_surface;
    GLFWwindow* m_window;
    uint32_t m_imageCount;
    std::vector<VkFramebuffer> m_framebuffers;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
};
