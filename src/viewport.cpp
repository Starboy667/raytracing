#include "viewport.hpp"

void Viewport::Render() {
    ImGui::Begin("Hello");
    ImGui::Button("Button");
    ImGui::End();

    ImGui::ShowDemoWindow();
}
