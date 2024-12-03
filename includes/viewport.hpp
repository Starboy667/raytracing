#pragma once
#include "imgui.h"

class Viewport {
   public:
    Viewport();
    ~Viewport() = default;

    void Render();
};
