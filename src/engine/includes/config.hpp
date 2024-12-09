#pragma once
#include <vector>

namespace config {
#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

// Other shared constants
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

// Validation layers
const inline std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

// Device extensions
const inline std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}