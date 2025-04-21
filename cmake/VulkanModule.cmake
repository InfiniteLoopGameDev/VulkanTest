find_package(Vulkan)

# Require Vulkan version ≥ 1.3.256 (earliest version when the Vulkan module was available)
if (${Vulkan_VERSION} VERSION_LESS "1.3.256")
    message(FATAL_ERROR "Minimum required Vulkan version for C++ modules is 1.3.256. "
            "Found ${Vulkan_VERSION}."
    )
endif ()

# set up Vulkan C++ module as a library
add_library(VulkanHppModule)
target_sources(VulkanHppModule PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS ${Vulkan_INCLUDE_DIR}
        FILES ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
)
target_compile_definitions(VulkanHppModule PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
target_compile_features(VulkanHppModule PUBLIC cxx_std_20)
target_link_libraries(VulkanHppModule PUBLIC Vulkan::Headers)