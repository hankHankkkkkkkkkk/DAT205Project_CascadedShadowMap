cmake_minimum_required(VERSION 3.16)

project(DAT205Project_CascadedShadowMap)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(extern/glfw)

add_executable(DAT205Project_CascadedShadowMap
    src/main.cpp
    src/shader.cpp
    extern/glad/src/glad.c
    extern/imgui/imgui.cpp
    extern/imgui/imgui_draw.cpp
    extern/imgui/imgui_tables.cpp
    extern/imgui/imgui_widgets.cpp
    extern/imgui/backends/imgui_impl_glfw.cpp
    extern/imgui/backends/imgui_impl_opengl3.cpp
 "src/camera.cpp" "src/camera.h" "src/mesh.h" "src/mesh.cpp" "src/render.h" "src/render.cpp" "src/csm.h" "src/csm.cpp" "src/shadowMap.h" "src/shadowMap.cpp" "src/gui.h" "src/gui.cpp" "src/gpuTimer.h" "src/gpuTimer.cpp")

add_custom_command(TARGET DAT205Project_CascadedShadowMap POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:DAT205Project_CascadedShadowMap>/assets
)

target_include_directories(DAT205Project_CascadedShadowMap PRIVATE
    src
    extern/glad/include
    extern
    extern/imgui
    extern/imgui/backends
)

target_link_libraries(DAT205Project_CascadedShadowMap PRIVATE
    glfw
    opengl32
)
