cmake_minimum_required(VERSION 3.16)

project(DAT205Project_CascadedShadowMap)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(extern/glfw)

add_executable(DAT205Project_CascadedShadowMap
    src/main.cpp
    src/shader.cpp
    extern/glad/src/glad.c
)

add_custom_command(TARGET DAT205Project_CascadedShadowMap POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:DAT205Project_CascadedShadowMap>/assets
)

target_include_directories(DAT205Project_CascadedShadowMap PRIVATE
    src
    extern/glad/include
    extern
)

target_link_libraries(DAT205Project_CascadedShadowMap PRIVATE
    glfw
    opengl32
)