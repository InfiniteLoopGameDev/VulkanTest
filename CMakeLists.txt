cmake_minimum_required(VERSION 3.25)

project(VulkanTest)

set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/.cache/cpm" CACHE STRING "CPM cache path")

include(cmake/CPM.cmake)

CPMFindPackage(
        NAME SFML
        GITHUB_REPOSITORY SFML/SFML
        GIT_TAG 3.0.1
        OPTIONS "SFML_BUILD_NETWORK OFF" "SFML_BUILD_AUDIO OFF" "SFML_BUILD_GRAPHICS OFF"
        "SFML_BUILD_WINDOW ON" "SFML_USE_STATIC_STD_LIBS OFF" "SFML_USE_SYSTEM_DEPS ON"
)

CPMFindPackage(
        NAME Vulkan
        GITHUB_REPOSITORY KhronosGroup/Vulkan-Headers
        GIT_TAG vulkan-sdk-1.4.321.0
)

include(cmake/FindSlang.cmake) # Custom CMake module to find the Slang compiler on system or through CPM
include(cmake/SlangUtils.cmake)

add_executable(VulkanTest)
target_compile_features(VulkanTest PUBLIC cxx_std_20)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(VulkanTest PROPERTIES WIN32_EXECUTABLE ON)
endif ()

set(VKT_SOURCES
        "src/main.cpp"
        "src/utils.cpp"
        "src/Application.cpp"
        "src/ApplicationSwapChainDetails.cpp"
        "src/ApplicationQueueFamilies.cpp"
)
set(VKT_HEADERS
        "src/utils.h"
        "src/Application.h"
        "src/ApplicationSwapChainDetails.h"
        "src/ApplicationQueueFamilies.h"
)
set(VKT_SLANG_SHADERS
        "shaders/triangle.slang"
)

target_sources(VulkanTest PRIVATE ${VKT_SOURCES} ${VKT_HEADERS})

add_shaders(VulkanTest ${VKT_SLANG_SHADERS})

target_link_libraries(VulkanTest PRIVATE Vulkan::Headers)

target_link_libraries(VulkanTest PRIVATE SFML::Window)

if (TARGET SFML::Main)
    target_link_libraries(VulkanTest PRIVATE SFML::Main)
endif ()
