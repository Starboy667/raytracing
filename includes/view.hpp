#pragma once
#include "imgui.h"

class View {
   public:
    virtual ~View() = default;
    virtual void Update() {}
    virtual void Render() {}
};
