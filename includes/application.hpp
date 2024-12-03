#pragma once

// clang-format off

#include "imgui.h"
#include "vulkan/vulkan.h"
#include "imgui_impl_vulkan.h"

struct GLFWwindow;
class Application {
   public:
    Application::Application()
	{
		Init();
	}
    ~Application() { Shutdown(); }
    void Run();

   private:
    int Init();
    void Shutdown();

   private:
	GLFWwindow* _window = nullptr;
};

Application* CreateApplication(int argc, char** argv);