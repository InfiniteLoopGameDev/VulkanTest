function(add_slang location)
    add_executable(slang::slangc IMPORTED)
    set_property(TARGET slang::slangc PROPERTY IMPORTED_LOCATION "${location}")
endfunction()


find_package(slang CONFIG QUIET)
if (slang_FOUND)
    if (NOT TARGET slang::slangc)
        add_slang(${SLANGC_EXECUTABLE})
    endif ()

    message(STATUS "Found Slang (Slang package): ${SLANGC_EXECUTABLE}")
    return()
endif ()

# Try to find the slangc executable on the system already
find_program(SLANGC_EXECUTABLE
        NAMES slangc
)
if (SLANGC_EXECUTABLE)
    add_slang(${SLANGC_EXECUTABLE})

    message(STATUS "Found Slang (system): ${SLANGC_EXECUTABLE}")
    return()
endif ()

# Vulkan SDK ships with it, useful if using system libraries
if (Vulkan_FOUND)
    if (DEFINED Vulkan::slangc)
        add_executable(slang::slangc ALIAS Vulkan::slangc)
        message(STATUS "Found Slang in Vulkan SDK")
        return()
    endif ()

    # CMake currently does not provide a target for slangc, so we have to find it manually
    get_target_property(_Vulkan_BIN Vulkan::glslc LOCATION)
    get_filename_component(_Vulkan_BIN_DIR ${_Vulkan_BIN} DIRECTORY)
    find_program(SLANGC_EXECUTABLE
            NAMES slangc
            PATHS ${_Vulkan_BIN_DIR}
    )
    if (SLANGC_EXECUTABLE)
        add_slang(${SLANGC_EXECUTABLE})

        message(STATUS "Found Slang (Vulkan SDK): ${SLANGC_EXECUTABLE}")
        return()
    endif ()
endif ()

if (MSVC)
    set(ADDITIONAL_ARGS
            "CMAKE_C_FLAGS_INIT -D_ITERATOR_DEBUG_LEVEL=0 /MP"
            "CMAKE_CXX_FLAGS_INIT -D_ITERATOR_DEBUG_LEVEL=0 /MP"
            "CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif ()

# If not found, use CPM to download and build slang from source
CPMAddPackage(
        NAME slang
        GITHUB_REPOSITORY shader-slang/slang
        GIT_TAG vulkan-sdk-1.4.321.0
        OPTIONS "SLANG_ENABLE_TESTS OFF" "SLANG_ENABLE_EXAMPLES OFF" "SLANG_ENABLE_GFX OFF" "SLANG_ENABLE_SLANGRT OFF"
        "SLANG_ENABLE_SLANGD OFF" "SLANG_ENABLE_SLANGI OFF" "SLANG_SLANG_LLVM_FLAVOR DISABLE" "SLANG_ENABLE_DXIL OFF"
        ${ADDITIONAL_ARGS}
)
# Quite hacky way to get the slangc executable from the build directory
# Has to be globed because the build directory is based on platform and version
if (TARGET slangc)
    add_executable(slang::slangc ALIAS slangc)
    add_dependencies(slangc slang-glslang)
else ()
    message(FATAL_ERROR "Could not find slangc")
endif ()