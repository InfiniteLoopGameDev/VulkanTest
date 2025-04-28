cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

if (CMAKE_VERSION VERSION_LESS 4.0)
        set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
else()
        set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
endif()

project(VulkanTest LANGUAGES CXX VERSION 0.0.1)

include(cmake/CPM.cmake)

CPMFindPackage(
        NAME SFML
        GITHUB_REPOSITORY SFML/SFML
        GIT_TAG 3.0.1
        OPTIONS "SFML_BUILD_EXAMPLES OFF" "SFML_BUILD_NETWORK OFF"
        "SFML_BUILD_AUDIO OFF" "SFML_BUILD_FREETYPE OFF" "SFML_BUILD_GRAPHICS OFF"
        "SFML_BUILD_WINDOW ON" "SFML_USE_STATIC_STD_LIBS OFF"
)

CPMFindPackage(
        NAME Vulkan
        GITHUB_REPOSITORY KhronosGroup/Vulkan-Headers
        GIT_TAG vulkan-sdk-1.4.309
        OPTIONS "VULKAN_HEADERS_ENABLE_MODULE ON" "VULKAN_HEADERS_ENABLE_MODULE_STD ON"
)

include(cmake/FindSlang.cmake) # Custom CMake module to find the Slang compiler on system or through CPM
include(cmake/VulkanModule.cmake) # Custom CMake module to add Vulkan as a C++ 20 module
include(cmake/SlangUtils.cmake)

add_executable(VulkanTest)
target_compile_features(VulkanTest PUBLIC cxx_std_23)
set_target_properties(VulkanTest PROPERTIES CXX_MODULE_STD ON)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(VulkanTest PROPERTIES WIN32_EXECUTABLE ON)
endif ()

set(VKT_SOURCES
        "src/main.cpp"
)

set(VKT_MODULES
        "src/utils.cppm"
        "src/ApplicationSwapChainDetails.cppm"
        "src/ApplicationQueueFamilies.cppm"
        "src/Application.cppm"
)

set(VKT_SLANG_SHADERS
        "shaders/triangle.slang"
)

target_sources(VulkanTest PRIVATE FILE_SET modules TYPE CXX_MODULES FILES ${VKT_MODULES})
target_sources(VulkanTest PRIVATE ${VKT_SOURCES} ${VKT_HEADERS})

add_shaders(VulkanTest ${VKT_SLANG_SHADERS})

target_link_libraries(VulkanTest PRIVATE Vulkan::HppModule)

target_link_libraries(VulkanTest PRIVATE SFML::Window)

if (TARGET SFML::Main)
    target_link_libraries(VulkanTest PRIVATE SFML::Main)
endif ()
