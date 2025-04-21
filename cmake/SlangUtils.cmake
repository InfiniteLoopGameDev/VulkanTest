# Modified from https://thatonegamedev.com/cpp/cmake/how-to-compile-shaders-with-cmake/#graphics-programming-with-vulkan

function(add_shaders TARGET_NAME)
    set(SHADER_SOURCE_FILES ${ARGN}) # the rest of arguments to this function will be assigned as shader source files

    # Validate that source files have been passed
    list(LENGTH SHADER_SOURCE_FILES FILE_COUNT)
    if (FILE_COUNT EQUAL 0)
        message(FATAL_ERROR "Cannot create a shaders target without any source files")
    endif ()

    set(SPIRV_DIR ${CMAKE_CURRENT_BINARY_DIR}/spirv)
    file(MAKE_DIRECTORY ${SPIRV_DIR})

    foreach (SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
        cmake_path(ABSOLUTE_PATH SHADER_SOURCE NORMALIZE)
        cmake_path(GET SHADER_SOURCE STEM SHADER_NAME)

        set(SLANG_OPTIMIZE_LEVEL 1 CACHE STRING "Slang compiler optimization level")
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(SLANG_OPTIMIZE_LEVEL 0)
        elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
            set(SLANG_OPTIMIZE_LEVEL 3)
        endif ()


        # Build command
        set(SHADER_COMMAND COMMAND ${SLANG_EXECUTABLE} -target spirv -O${SLANG_OPTIMIZE_LEVEL}
                -fvk-use-entrypoint-name -source-embed-name ${SHADER_NAME}
                -source-embed-style u32 -o ${SPIRV_DIR}/${SHADER_NAME}.spv
                ${SHADER_SOURCE})

        add_custom_command(
                OUTPUT "${SPIRV_DIR}/${SHADER_NAME}.spv.h"
                COMMENT "Compiling ${SHADER_NAME}.slang to SPIR-V"
                COMMAND ${SHADER_COMMAND}
                DEPENDS ${SHADER_SOURCE}
        )

        add_custom_target(${SHADER_NAME} DEPENDS ${SPIRV_DIR}/${SHADER_NAME}.spv.h)

        add_dependencies(${TARGET_NAME} ${SHADER_NAME})
    endforeach ()

    target_include_directories(${TARGET_NAME} PRIVATE ${SPIRV_DIR})
endfunction()