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
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

# Vulkan Memory Allocator
FetchContent_Declare(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.1.0
)
FetchContent_MakeAvailable(glfw glm vma)

# Executable Target
file(GLOB_RECURSE SRC_FILES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    project_warnings
    Vulkan::Vulkan
    Vulkan::Headers
    glfw
    glm::glm
    VulkanMemoryAllocator
)

# Shader Compilation Pipeline
set(SPV_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY ${SPV_OUTPUT_DIR})

set(SHADER_SOURCES
    shaders/compute/spatial_hash.comp
    shaders/compute/sph_density.comp
    shaders/compute/sph_forces.comp
    shaders/compute/thermal_advection.comp
    shaders/graphics/particle.vert
    shaders/graphics/particle.frag
    shaders/graphics/cad_mesh.vert
    shaders/graphics/cad_mesh.frag
)

set(SPV_OUTPUTS "")
foreach(SHADER ${SHADER_SOURCES})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}")
        get_filename_component(SHADER_NAME %{SHADER} NAME)
        set(SPV_FILE "${SPV_OUTPUT_DIR}/${SHADER_NAME}.spv")

        add_custom_command(
            OUTPUT ${SPV_FILE}
            COMMAND Vulkan::glslc --target-env=vulkan1.3 -0 "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}" -o "${SPV_FILE}"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}"
            COMMENT "Compiling ${SHADER} -> ${SHADER_NAME}.spv"
            VERBATIM
        )
        list(APPEND SPV_OUTPUTS ${SPV_FILE})
    endif()
endforeach()

if(SPV_OUTPUTS)
    add_custom_target(compile_shaders ALL DEPENDS ${SPV_OUTPUTS})
    add_dependencies(${PROJECT_NAME} compile_shaders)
endif()