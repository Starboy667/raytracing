#pragma once

// clang-format off
#include <vector>
#include <memory>
#include <functional> 

#include "view.hpp"
#include "imgui.h"
#include "vulkan/vulkan.h"
#include "imgui_impl_vulkan.h"

void check_vk_result(VkResult err);

struct GLFWwindow;
class Application {
   public:
    Application::Application()
	{
		Init();
	}
    ~Application() { Shutdown(); }
    void Run();
    template<typename T>
    void AddView() {
        static_assert(std::is_base_of<View, T>::value, "Pushed type is not subclass of View!");
        _viewStack.emplace_back(std::make_shared<T>());
    }
    void PushView(const std::shared_ptr<View>& layer) { _viewStack.emplace_back(layer); }
    static VkDevice GetDevice();
    static VkPhysicalDevice GetPhysicalDevice();
    static VkCommandBuffer GetCommandBuffer(bool begin);
    static void FlushCommandBuffer(VkCommandBuffer commandBuffer);
    static void SubmitResourceFree(std::function<void()>&& func);

   private:
    int Init();
    void Shutdown();

   private:
	GLFWwindow* _window = nullptr;
    std::vector<std::shared_ptr<View>> _viewStack;
};

Application* CreateApplication(int argc, char** argv);