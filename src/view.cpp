#include "view.hpp"

#include "application.hpp"
#include "image.hpp"

class Viewport : public View {
   public:
    virtual void RenderUI() override {
        {
            ImGui::Begin("Settings");
            if (ImGui::Button("Render")) {
                Render();
            }
            ImGui::End();
            // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
            //                     ImVec2(0.0f, 0.0f));
            ImGui::Begin("Viewport");
            // display image
            _viewPortWidth =
                static_cast<uint32_t>(ImGui::GetContentRegionAvail().x);
            _viewPortHeight =
                static_cast<uint32_t>(ImGui::GetContentRegionAvail().y);

            if (_image) {
                ImGui::Image(
                    (ImTextureID)_image->GetDescriptorSet(),
                    {(float)_image->GetWidth(), (float)_image->GetHeight()});
            }
            ImGui::End();
            // ImGui::PopStyleVar();
        }
    }

    void Render() {
        if (!_image || _viewPortWidth != _image->GetWidth() ||
            _viewPortHeight != _image->GetHeight()) {
            _image = std::make_shared<Image>(_viewPortWidth, _viewPortHeight,
                                             ImageFormat::RGBA);
            delete[] _imageData;
            _imageData = new uint32_t[_viewPortWidth * _viewPortHeight];
        }
        for (uint32_t i = 0; i < _viewPortWidth * _viewPortHeight; i++) {
            _imageData[i] = 0xffff00ff;
        }
        _image->SendData(_imageData);
    }
    virtual void Update() override {}

   private:
    std::shared_ptr<Image> _image;
    uint32_t* _imageData = nullptr;
    uint32_t _viewPortWidth = 0;
    uint32_t _viewPortHeight = 0;
};

Application* CreateApplication(int argc, char** argv) {
    Application* app = new Application();
    app->AddView<Viewport>();
    return app;
}