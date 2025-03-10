cmake_minimum_required(VERSION 3.25)

project(VulkanTest)

find_package(SFML REQUIRED CONFIG COMPONENTS Window)

find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE VKT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
file(GLOB_RECURSE VKT_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.h")

add_executable(VulkanTest ${VKT_SOURCES})

target_link_libraries(VulkanTest PUBLIC Vulkan::Vulkan)

target_link_libraries(VulkanTest PRIVATE SFML::Window)
