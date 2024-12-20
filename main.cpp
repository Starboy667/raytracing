#include "application.hpp"
#include "src/engine/engine.hpp"

int main() {
    try {
        Application app(1280, 720, "Raytracing");
        app.Run();
    } catch (const std::exception& e) {
        printf("%s\n", e.what());
        return 1;
    }

    return 0;
}