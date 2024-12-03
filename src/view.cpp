#include "view.hpp"

#include "application.hpp"

class Viewport : public View {
   public:
    virtual void Render() override {
        {
            ImGui::Begin("Viewport");
            ImGui::Button("Button");
            ImGui::End();
        }
    }

    virtual void Update() override {}
};

Application* CreateApplication(int argc, char** argv) {
    Application* app = new Application();
    app->AddView<Viewport>();
    return app;
}