cmake_minimum_required(VERSION 3.24)
project(AerothermalSim LANGUAGES CXX)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clangd / language servers
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler Warning Interface
add_library(project_warnings INTERFACE)
if (MSVC)
    target_compile_options(project_warnings INTERFACE
        /W4 # Warning level
        /permissive- # Disables  non-standard extensions
        /w14640 # Warn on unthreadsafe local static initialization
    )
else()
    target_compile_options(project_warnings INTERFACE 
        -Wall # Enables a set of warning messages
        -Wextra # Enables extra warning flags not covered in -Wall
        -pedantic # For warnings by ISO C and ISO C++
        -Wconversion # Warns for implicit type conversion that may alter a value
        -Wsign-conversion # Warns for implicit conversions that changes signed of a value, like int to unsigned int
        -Wnon-virtual-dtor 
        -Wold-style-cast
    )
endif()

# Dependencies
find_package(Vulkan REQUIRED COMPONENTS glslc)

include(FetchContent)

# GLFW (Windowing and Input)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

# GLM (Header-Only Math)
FetchContent(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

# Vulkan Memory Allocator

