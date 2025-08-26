# Try to find the slangc executable on the system already
# Vulkan SDK ships with it, useful if using system libraries
if (NOT Vulkan_FOUND)
    find_package(Vulkan)
endif ()

if (DEFINED Vulkan::slangc)
    set(SLANG_EXECUTABLE Vulkan::slangc)
else ()
    # CMake currently does not provide a target for slangc, so we have to find it manually
    get_target_property(_Vulkan_LIB_DIR Vulkan::glslc LOCATION)
    get_filename_component(_Vulkan_LIB_DIR ${_Vulkan_LIB_DIR} DIRECTORY)
    find_program(SLANGC_EXECUTABLE
            NAMES slangc
            PATHS ${_Vulkan_LIB_DIR}
    )
endif ()

if (SLANGC_EXECUTABLE)
    message(STATUS "Found Slang (Vulkan SDK): ${SLANGC_EXECUTABLE}")
    return()
endif ()


# If not found, use CPM to download and build slang from source
CPMFindPackage(
        NAME slang
        GITHUB_REPOSITORY shader-slang/slang
        GIT_TAG vulkan-sdk-1.4.321.0
        OPTIONS "SLANG_ENABLE_TESTS OFF" "SLANG_ENABLE_EXAMPLES OFF" "SLANG_ENABLE_GFX OFF" "SLANG_ENABLE_SLANGRT OFF" "SLANG_USE_SYSTEM_VULKAN_HEADERS ON"
)
# Quite hacky way to get the slangc executable from the build directory
# Has to be globed because the build directory is based on platform and version
if (NOT SLANGC_EXECUTABLE)
    file(GLOB SLANG_BINARIES "${slang_BINARY_DIR}/slang*/bin")
    find_program(SLANGC_EXECUTABLE
            NAMES slangc
            PATHS ${SLANG_BINARIES}
            NO_DEFAULT_PATH
            REQUIRED
    )
endif ()