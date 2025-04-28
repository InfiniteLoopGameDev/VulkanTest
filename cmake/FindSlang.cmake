# Try to find the slangc executable on the system already
# Vulkan ships it, CPM does not check for it
find_package(Vulkan)
if (DEFINED Vulkan_SLANGC_EXECUTABLE)
    set(SLANG_EXECUTABLE ${Vulkan_SLANGC_EXECUTABLE})
    message(STATUS "Found Slang: ${SLANGC_EXECUTABLE}")
    return()
endif ()
get_filename_component(_Vulkan_LIB_DIR ${Vulkan_LIBRARY} DIRECTORY)
find_program(SLANGC_EXECUTABLE
        NAMES slangc
        HINTS ${_Vulkan_LIB_DIR}/../Bin
)
if (SLANGC_EXECUTABLE)
    message(STATUS "Found Slang: ${SLANGC_EXECUTABLE}")
    return()
endif ()

find_package(slang QUIET)
if (NOT SLANGC_EXECUTABLE)
    CPMAddPackage(
            NAME slang
            GITHUB_REPOSITORY shader-slang/slang
            GIT_TAG vulkan-sdk-1.4.309
            OPTIONS "SLANG_ENABLE_TESTS OFF" "SLANG_ENABLE_EXAMPLES OFF" "SLANG_ENABLE_GFX OFF" "SLANG_ENABLE_SLANGRT OFF" "SLANG_USE_SYSTEM_VULKAN_HEADERS ON"
    )
    # Quite hacky way to get the slangc executable from the build directory
    # Has to be globed because the build directory is based on platform and version
    file(GLOB SLANG_BINARIES "${slang_BINARY_DIR}/slang*/bin")
    find_program(SLANGC_EXECUTABLE
            NAMES slangc
            PATHS ${SLANG_BINARIES}
            NO_DEFAULT_PATH
            REQUIRED
    )
endif ()