cmake_minimum_required(VERSION 3.25)

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

set_property(TARGET VulkanTest PROPERTY CXX_STANDARD 23)

file(GLOB_RECURSE VKT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
file(GLOB_RECURSE VKT_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.h")

target_sources(VulkanTest PRIVATE ${VKT_SOURCES} ${VKT_HEADERS})

target_link_libraries(VulkanTest PRIVATE Vulkan::Headers)

target_link_libraries(VulkanTest PRIVATE SFML::Window)
