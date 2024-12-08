#include "application.hpp"
#include "engine/engine.hpp"

int main() {
    try {
        Application app(800, 600, "Raytracing");
        app.Run();
    } catch (const std::exception& e) {
        printf("%s\n", e.what());
        return 1;
    }

    return 0;
}