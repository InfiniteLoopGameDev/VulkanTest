cmake_minimum_required(VERSION 3.25)

#if (CMAKE_BUILD_TYPE STREQUAL "Debug")
set(VCPKG_MANIFEST_FEATURES "validation")
#endif ()

project(VulkanTest)

# 1. Look for a SDL2 package, 2. look for the SDL2 component and 3. fail if none can be found
find_package(SDL2 REQUIRED CONFIG)

# 1. Look for a SDL2 package, 2. Look for the SDL2maincomponent and 3. DO NOT fail when SDL2main is not available
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)

set(SDL2PP_STATIC ON)

add_subdirectory(thirdparty/SDL2pp)

find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE VKT_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.hpp")
file(GLOB_RECURSE VKT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")

set(EXE_OPTS "")

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    #list(APPEND EXE_OPTS WIN32)
endif ()

add_executable(VulkanTest ${EXE_OPTS} ${VKT_SOURCES})

# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications
if (TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(VulkanTest PRIVATE SDL2::SDL2main)
endif ()

target_link_libraries(VulkanTest PUBLIC Vulkan::Vulkan)

target_link_libraries(VulkanTest PRIVATE SDL2::SDL2)

target_link_libraries(VulkanTest PRIVATE SDL2pp::SDL2pp)