#include "application.hpp"
#include "engine/engine.hpp"

int main() {
    Application app(800, 600, "Raytracing");

    try {
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}