#include "application.hpp"
#include "engine/engine.hpp"

int main() {
    Application app(800, 600, "Raytracing");

    try {
        app.Run();
    } catch (const std::exception& e) {
        printf("%s\n", e.what());
        return 1;
    }

    return 0;
}