if (CPM_USE_LOCAL_PACKAGES)
    # Try to find the slangc executable on the system already
    # Vulkan ships it, CPM does not check for it
    find_package(Vulkan)
    if (DEFINED Vulkan_SLANGC_EXECUTABLE)
        set(SLANG_EXECUTABLE ${Vulkan_SLANGC_EXECUTABLE})
        message(STATUS "Found Slang: ${Vulkan_SLANGC_EXECUTABLE}")
        return()
    endif ()
    get_filename_component(_Vulkan_LIB_DIR ${Vulkan_LIBRARY} DIRECTORY)
    find_program(SLANG_EXECUTABLE
            NAMES slangc
            HINTS ${_Vulkan_LIB_DIR}/../Bin
    )
    message(STATUS "Found Slang: ${Vulkan_SLANGC_EXECUTABLE}")
    return()
endif ()

CPMAddPackage(
        NAME slang
        GITHUB_REPOSITORY shader-slang/slang
        GIT_TAG vulkan-sdk-1.4.309
)