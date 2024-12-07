# Example usage:
#  mkdir build
#  cd build
#  cmake -g "Visual Studio 14 2015" ..

cmake_minimum_required(VERSION 3.10)

project(raytracer)

# Sources
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS includes/*.hpp includes/*.h)

# Create the executable target first
add_executable(raytracer ${SOURCES} ${HEADERS} ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui_impl_glfw.cpp ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui_impl_vulkan.cpp ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui.cpp ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui_draw.cpp ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui_demo.cpp ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui_tables.cpp ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui_widgets.cpp)

# C++ 17
set_target_properties(raytracer PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

# Includes
target_include_directories(raytracer PUBLIC includes)

# linker flags
add_link_options(-lmingw32 -lgdi32 -luser32 -lkernel32 -ladvapi32 -lshell32 -lgcc -lmoldname -lmingwex -lmsvcrt -lwinspool -lcomdlg32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

if(MSVC)
    # Force to always compile with /MD or /MDd
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    
    # Disable the MSVCRT conflict warning
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
    
    # Add runtime library flags
    target_compile_options(raytracer PRIVATE 
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
    )
endif()

# Define the paths for the shader sources
set(COMP_SHADER "${CMAKE_SOURCE_DIR}/res/shaders/shader.comp")

# Define the output paths for SPIR-V files
set(SPIRV_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/res/shaders")
set(COMP_SPIRV "${SPIRV_OUTPUT_DIR}/comp.spv")

# Ensure the output directory exists
file(MAKE_DIRECTORY ${SPIRV_OUTPUT_DIR})

set(GLSLC_PATH "C:/VulkanSDK/1.3.296.0/Bin/glslc.exe")
# Add custom command to compile the shaders
add_custom_command(
    OUTPUT ${COMP_SPIRV}
    COMMAND ${GLSLC_PATH} ${COMP_SHADER} -o ${COMP_SPIRV}
    DEPENDS ${COMP_SHADER}
    VERBATIM
)


# Add a custom target to ensure shaders are part of the build process
add_custom_target(CompileShaders ALL DEPENDS ${COMP_SPIRV})
# Ensure raytracer depends on the compiled shaders
add_dependencies(raytracer CompileShaders)

# GLFW
add_library(glfw STATIC IMPORTED)
set(GLFW_DIR lib/glfw)
set_target_properties(glfw PROPERTIES IMPORTED_LOCATION  ${PROJECT_SOURCE_DIR}/lib/glfw/lib-vc2022/glfw3.lib)
target_include_directories(glfw INTERFACE ${GLFW_DIR}/include)

# Dear ImGui
set(IMGUI_DIR ../../)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)

# GLM
# include_directories(${PROJECT_SOURCE_DIR}/lib/glm)
 # Add GLM include directory
include_directories(${PROJECT_SOURCE_DIR}/lib)
 
# STB
include_directories(${PROJECT_SOURCE_DIR}/lib/stb)

# Libraries
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
set(LIBRARIES "glfw;Vulkan::Vulkan")

# Use vulkan headers from glfw:
include_directories(${GLFW_DIR}/deps)
include_directories(${PROJECT_SOURCE_DIR}/lib/ImGui)

target_link_libraries(raytracer ${LIBRARIES})