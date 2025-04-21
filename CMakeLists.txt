cmake_minimum_required(VERSION 3.28)

project(VulkanTest)

set(CPM_USE_LOCAL_PACKAGES ON)
include(cmake/CPM.cmake)

CPMAddPackage(
        NAME SFML
        GITHUB_REPOSITORY SFML/SFML
        GIT_TAG 3.0.0
        OPTIONS "SFML_BUILD_EXAMPLES OFF" "SFML_BUILD_NETWORK OFF"
        "SFML_BUILD_AUDIO OFF" "SFML_BUILD_FREETYPE OFF" "SFML_BUILD_GRAPHICS OFF"
        "SFML_BUILD_WINDOW ON" "SFML_USE_STATIC_STD_LIBS OFF"
)

CPMAddPackage(
        NAME Vulkan
        GITHUB_REPOSITORY KhronosGroup/Vulkan-Headers
        GIT_TAG vulkan-sdk-1.4.309
        OPTIONS "VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=ON"
)

include(cmake/FindSlang.cmake) # Custom CMake module to find the Slang compiler on system or through CPM
include(cmake/SlangUtils.cmake)

add_executable(VulkanTest)
target_compile_features(VulkanTest PUBLIC cxx_std_20)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(VulkanTest PROPERTIES WIN32_EXECUTABLE ON)
endif ()
set_property(TARGET VulkanTest PROPERTY CXX_SCAN_FOR_MODULES ON)

set(VKT_SOURCES
        "src/main.cpp"
        "src/utils.cpp"
        "src/Application.cpp"
)
set(VKT_HEADERS
        "src/utils.h"
        "src/Application.h"
        "src/SwapChainDetails.h"
        "src/QueueFamilyIndices.h"
)
set(VKT_SLANG_SHADERS
        "shaders/triangle.slang"
)

target_sources(VulkanTest PRIVATE FILE_SET modules TYPE CXX_MODULES FILES ${VKT_MODULES})
target_sources(VulkanTest PRIVATE ${VKT_SOURCES} ${VKT_HEADERS})

add_shaders(VulkanTest ${VKT_SLANG_SHADERS})

target_link_libraries(VulkanTest PRIVATE Vulkan::Headers)

target_link_libraries(VulkanTest PRIVATE SFML::Window SFML::Main)
