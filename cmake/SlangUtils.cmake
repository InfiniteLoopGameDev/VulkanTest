# Modified from https://thatonegamedev.com/cpp/cmake/how-to-compile-shaders-with-cmake/#graphics-programming-with-vulkan

function(slang_compile_spirv)
    set(oneValueArgs TARGET_NAME)
    set(multiValueArgs SOURCE_FILES CAPABILITIES)
    cmake_parse_arguments(ADD_SHADER "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Validate that source files have been passed
    message(STATUS)
    list(LENGTH ADD_SHADER_SOURCE_FILES FILE_COUNT)
    if (FILE_COUNT EQUAL 0)
        message(FATAL_ERROR "Cannot create a shaders target without any source files")
    endif ()

    set(SPIRV_DIR ${CMAKE_CURRENT_BINARY_DIR}/spirv)
    file(MAKE_DIRECTORY ${SPIRV_DIR})

    foreach (SHADER_SOURCE IN LISTS ADD_SHADER_SOURCE_FILES)
        cmake_path(ABSOLUTE_PATH SHADER_SOURCE NORMALIZE)
        cmake_path(GET SHADER_SOURCE STEM SHADER_NAME)

        set(SLANG_OPTIMIZE_LEVEL 1 CACHE STRING "Slang compiler optimization level")
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(SLANG_OPTIMIZE_LEVEL 0)
        elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
            set(SLANG_OPTIMIZE_LEVEL 3)
        endif ()

        # Join the capabilities into a single string
        string(JOIN "+" CAPABILITIES "${ADD_SHADER_CAPABILITIES}")

        # Build command
        set(SHADER_COMMAND COMMAND slang::slangc -target spirv -O${SLANG_OPTIMIZE_LEVEL}
                -fvk-use-entrypoint-name -source-embed-name ${SHADER_NAME}
                -capability ${SHADER_CAPABILITIES} -source-embed-style u32
                -o ${SPIRV_DIR}/${SHADER_NAME}.spv
                ${SHADER_SOURCE})

        add_custom_command(
                OUTPUT "${SPIRV_DIR}/${SHADER_NAME}.spv.h"
                COMMENT "Building SPIR-V object ${SHADER_SOURCE}"
                COMMAND ${SHADER_COMMAND}
                DEPENDS ${SHADER_SOURCE}
                WORKING_DIRECTORY $<TARGET_FILE_DIR:slang::slangc>
        )

        add_custom_target(${SHADER_NAME} DEPENDS ${SPIRV_DIR}/${SHADER_NAME}.spv.h)

        add_dependencies(${ADD_SHADER_TARGET_NAME} ${SHADER_NAME})
    endforeach ()

    target_include_directories(${ADD_SHADER_TARGET_NAME} PRIVATE ${SPIRV_DIR})
endfunction()