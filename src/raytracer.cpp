#include "application.hpp"

Application* CreateApplication(int argc, char** argv) {
    return new Application();
}