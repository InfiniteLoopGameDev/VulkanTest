cmake_minimum_required(VERSION 3.25)

project(VulkanTest)

# Download xrepo.cmake if not exists in build directory.
if (NOT EXISTS "${CMAKE_BINARY_DIR}/xrepo.cmake")
    message(STATUS "Downloading xrepo.cmake from https://github.com/xmake-io/xrepo-cmake/")
    # mirror https://cdn.jsdelivr.net/gh/xmake-io/xrepo-cmake@main/xrepo.cmake
    file(DOWNLOAD "https://raw.githubusercontent.com/xmake-io/xrepo-cmake/main/xrepo.cmake"
            "${CMAKE_BINARY_DIR}/xrepo.cmake"
            TLS_VERIFY ON)
endif ()

# Include xrepo.cmake so we can use xrepo_package function.
include(${CMAKE_BINARY_DIR}/xrepo.cmake)

xrepo_package("sfml >=3.0.0" MODE debug CONFIGS "vs_runtime='MDd'")

set(SFML_STATIC_LIBRARIES ON)

find_package(SFML REQUIRED CONFIG COMPONENTS Window)

xrepo_package("vulkan-headers >=1.2.0")

find_package(VulkanHeaders REQUIRED CONFIG)

add_executable(VulkanTest)

set_property(TARGET VulkanTest PROPERTY CXX_STANDARD 23)

file(GLOB_RECURSE VKT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
file(GLOB_RECURSE VKT_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.h")

target_sources(VulkanTest PRIVATE ${VKT_SOURCES} ${VKT_HEADERS})

target_link_libraries(VulkanTest PRIVATE Vulkan::Headers)

target_link_libraries(VulkanTest PRIVATE SFML::Window)
