#include "application.hpp"

int main(int argc, char** argv) {
    Application* app = CreateApplication(argc, argv);
    app->Run();
    delete app;
    return 0;
}