cmake_minimum_required(VERSION 3.25)

project(VulkanTest)

# 1. Look for a SDL2 package, 2. look for the SDL2 component and 3. fail if none can be found
find_package(SFML REQUIRED CONFIG COMPONENTS window)

find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE VKT_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.hpp")
file(GLOB_RECURSE VKT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")

set(EXE_OPTS "")

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND EXE_OPTS WIN32)
endif()

add_executable(VulkanTest ${EXE_OPTS} ${VKT_SOURCES})

target_link_libraries(VulkanTest PUBLIC Vulkan::Vulkan)

target_link_libraries(VulkanTest PRIVATE sfml-window)
