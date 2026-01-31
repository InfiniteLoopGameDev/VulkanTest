cmake_minimum_required(VERSION 3.25)

project(VulkanTest)

option(WAYLAND "Enable Wayland Support" ON)
option(XLIB "Enable X11/XLib Support" ON)

set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/.cache/cpm" CACHE STRING "CPM cache path")

include(cmake/CPM.cmake)

CPMFindPackage(
        NAME SFML
        GITHUB_REPOSITORY SFML/SFML
        GIT_TAG 3.0.2
        OPTIONS "SFML_BUILD_NETWORK OFF" "SFML_BUILD_AUDIO OFF" "SFML_BUILD_GRAPHICS OFF"
        "SFML_BUILD_WINDOW ON" "SFML_USE_SYSTEM_DEPS ON"
)

CPMFindPackage(
        NAME Vulkan
        GITHUB_REPOSITORY KhronosGroup/Vulkan-Headers
        GIT_TAG vulkan-sdk-1.4.321.0
)

if (WIN32)
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)
elseif (LINUX)
    if (WAYLAND)
        add_compile_definitions(VK_USE_PLATFORM_WAYLAND_KHR)
    elseif (XLIB)
        add_compile_definitions(VK_USE_PLATFORM_XLIB_KHR)
    else ()
        message(FATAL_ERROR "Need at least one display protocol")
    endif ()
elseif (APPLE)
    if (IOS)
        add_compile_definitions(VK_USE_PLATFORM_IOS_MVK)
    else ()
        add_compile_definitions(VK_USE_PLATFORM_MACOS_MVK)
    endif ()
endif ()

CPMFindPackage(
        NAME glm
        GITHUB_REPOSITORY g-truc/glm
        GIT_TAG 1.0.1
        OPTIONS "GLM_BUILD_TESTS OFF"
)

include(cmake/FindSlang.cmake) # Custom CMake module to find the Slang compiler on system or through CPM
include(cmake/SlangUtils.cmake)

add_executable(VulkanTest)
target_compile_features(VulkanTest PRIVATE cxx_std_20)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(VulkanTest PROPERTIES WIN32_EXECUTABLE ON)
endif ()

set(VKT_SOURCES
        "src/main.cpp"
        "src/utils.cpp"
        "src/Application.cpp"
        "src/ApplicationSwapChainDetails.cpp"
        "src/ApplicationQueueFamilies.cpp"
        "src/Vertex.cpp"
        "src/Window/Win32Window.cpp"
)
set(VKT_HEADERS
        "src/utils.h"
        "src/Application.h"
        "src/ApplicationSwapChainDetails.h"
        "src/ApplicationQueueFamilies.h"
        "src/Vertex.h"
        "src/Window/Window.h"
        "src/Window/Win32Window.h"
)
set(VKT_SLANG_SHADERS
        "shaders/triangle.slang"
)

target_sources(VulkanTest PRIVATE ${VKT_SOURCES} ${VKT_HEADERS})

set(SHADER_CAPABILITIES
        "spirv_1_5"
)
slang_compile_spirv(
        TARGET_NAME VulkanTest
        SOURCE_FILES ${VKT_SLANG_SHADERS}
        CAPABILITIES ${SHADER_CAPABILITIES}
)

target_link_libraries(VulkanTest PRIVATE Vulkan::Headers)

target_link_libraries(VulkanTest PRIVATE SFML::Window)

if (TARGET SFML::Main)
    target_link_libraries(VulkanTest PRIVATE SFML::Main)
endif ()

target_link_libraries(VulkanTest PRIVATE glm::glm)

if (BUILD_SHARED_LIBS)
    add_custom_command(TARGET VulkanTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:VulkanTest>
            $<TARGET_FILE_DIR:VulkanTest>
            COMMAND_EXPAND_LISTS
    )
endif ()