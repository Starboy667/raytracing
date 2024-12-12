# linker flags
# add_link_options(-lmingw32 -lgdi32 -luser32 -lkernel32 -ladvapi32 -lshell32 -lgcc -lmoldname -lmingwex -lmsvcrt -lwinspool -lcomdlg32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32)


cmake_minimum_required(VERSION 3.13.4)
project(raytracer)

set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
# set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

file(GLOB_RECURSE SOURCES main.cpp ${PROJECT_SOURCE_DIR}/src/*.cpp)

set(ImGuiVulkan  ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui_impl_glfw.cpp ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui_impl_vulkan.cpp ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui.cpp ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui_draw.cpp ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui_demo.cpp ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui_tables.cpp ${PROJECT_SOURCE_DIR}/lib/ImGui/imgui_widgets.cpp)
add_executable(raytracer ${SOURCES} ${HEADERS} ${ImGuiVulkan})

set_target_properties(raytracer PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_include_directories(raytracer PUBLIC includes)

if(MSVC)
    # Force to always compile with /MD or /MDd
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    # Disable the MSVCRT conflict warning
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
    # Add runtime library flags
    add_compile_options(/W4 /WX)
    target_compile_options(raytracer PRIVATE 
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
    )
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# shaders
set(COMP_SHADER "${CMAKE_SOURCE_DIR}/res/shaders/shader.comp")
set(SPIRV_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/res/shaders")
set(COMP_SPIRV "${SPIRV_OUTPUT_DIR}/comp.spv")
file(MAKE_DIRECTORY ${SPIRV_OUTPUT_DIR})
set(GLSLC_PATH "C:/VulkanSDK/1.3.296.0/Bin/glslc.exe")
add_custom_command(
    OUTPUT ${COMP_SPIRV}
    COMMAND ${GLSLC_PATH} ${COMP_SHADER} -o ${COMP_SPIRV}
    DEPENDS ${COMP_SHADER}
    VERBATIM
)
add_custom_target(CompileShaders ALL DEPENDS ${COMP_SPIRV})
add_dependencies(raytracer CompileShaders)

# include glfw
add_library(glfw STATIC IMPORTED)
set(GLFW_DIR lib/glfw)
set_target_properties(glfw PROPERTIES IMPORTED_LOCATION  ${PROJECT_SOURCE_DIR}/lib/glfw/lib-vc2022/glfw3.lib)
target_include_directories(glfw INTERFACE ${GLFW_DIR}/include)
# include glm headers
include_directories(${PROJECT_SOURCE_DIR}/lib/glm)
# include stb headers
include_directories(${PROJECT_SOURCE_DIR}/lib/stb)
# include imgui
include_directories(${PROJECT_SOURCE_DIR}/lib/ImGui)
add_subdirectory(lib/yaml_cpp)   
include_directories(
   lib/yaml_cpp/include
) 
# include vulkan headers from glfw
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
set(LIBRARIES "glfw;Vulkan::Vulkan")

target_link_libraries(raytracer ${LIBRARIES} yaml-cpp)