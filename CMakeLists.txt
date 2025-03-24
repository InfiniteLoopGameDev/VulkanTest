cmake_minimum_required(VERSION 3.28)

project(VulkanTest)

find_package(SFML REQUIRED CONFIG COMPONENTS Window)

find_package(VulkanHeaders REQUIRED CONFIG)

add_executable(VulkanTest)

set_property(TARGET VulkanTest PROPERTY CXX_STANDARD 23)
set_property(TARGET VulkanTest PROPERTY CXX_SCAN_FOR_MODULES ON)

file(GLOB_RECURSE VKT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
file(GLOB_RECURSE VKT_MODULES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cppm")
file(GLOB_RECURSE VKT_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.h")

target_sources(VulkanTest PRIVATE FILE_SET modules TYPE CXX_MODULES FILES ${VKT_MODULES})
target_sources(VulkanTest PRIVATE ${VKT_SOURCES} ${VKT_HEADERS})

target_link_libraries(VulkanTest PRIVATE Vulkan::Headers)

target_link_libraries(VulkanTest PRIVATE SFML::Window)
