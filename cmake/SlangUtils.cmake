# Modified from https://thatonegamedev.com/cpp/cmake/how-to-compile-shaders-with-cmake/#graphics-programming-with-vulkan

function(slang_compile_spirv)
    set(oneValueArgs TARGET_NAME)
    set(multiValueArgs SOURCE_FILES CAPABILITIES)
    cmake_parse_arguments(ADD_SHADER "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Validate that source files have been passed
    list(LENGTH ADD_SHADER_SOURCE_FILES FILE_COUNT)
    if (FILE_COUNT EQUAL 0)
        message(FATAL_ERROR "Cannot create a shaders target without any source files")
    endif ()

    add_custom_target(Shaders)

    set(SLANG_OPTIMIZE_LEVEL 1 CACHE STRING "Slang compiler optimization level")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(SLANG_OPTIMIZE_LEVEL 0)
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        set(SLANG_OPTIMIZE_LEVEL 3)
    endif ()

    cmake_path(APPEND CMAKE_CURRENT_BINARY_DIR "spirv" OUTPUT_VARIABLE SPIRV_DIR)
    file(MAKE_DIRECTORY ${SPIRV_DIR})
    cmake_path(APPEND SPIRV_DIR "include" OUTPUT_VARIABLE INCLUDE_DIR)
    file(MAKE_DIRECTORY ${INCLUDE_DIR})

    foreach (SHADER_SOURCE IN LISTS ADD_SHADER_SOURCE_FILES)
        cmake_path(ABSOLUTE_PATH SHADER_SOURCE NORMALIZE)
        cmake_path(GET SHADER_SOURCE STEM SHADER_NAME)

        # Join the capabilities into a single string
        string(JOIN "+" CAPABILITIES ${ADD_SHADER_CAPABILITIES})

        cmake_path(APPEND SPIRV_DIR "${SHADER_NAME}.spv" OUTPUT_VARIABLE SHADER_OUTPUT)

        # Build command
        set(SHADER_COMMAND_STRING COMMAND slang::slangc -target spirv -O${SLANG_OPTIMIZE_LEVEL}
                -fvk-use-entrypoint-name -source-embed-name ${SHADER_NAME}
                -capability ${CAPABILITIES} -source-embed-style u32
                -o ${SHADER_OUTPUT}.c.h
                ${SHADER_SOURCE})

        add_custom_command(
                OUTPUT ${SHADER_OUTPUT}.c.h
                COMMENT "Building SPIR-V object ${SHADER_OUTPUT}"
                COMMAND ${SHADER_COMMAND_STRING}
                DEPENDS ${SHADER_SOURCE}
                WORKING_DIRECTORY $<TARGET_FILE_DIR:slang::slangc>
        )
        set_source_files_properties(${SHADER_OUTPUT}.c.h PROPERTIES LANGUAGE C)

        cmake_path(APPEND INCLUDE_DIR "${SHADER_NAME}.h" OUTPUT_VARIABLE SHADER_HEADER)
        set(SHADER_HEADER_OUTPUT
                "#pragma once\n"
                "#include <stdint.h>\n"
                "extern \"C\" {\n"
                "extern const uint32_t ${SHADER_NAME}[]\;\n"
                "extern const size_t ${SHADER_NAME}_sizeInBytes\;\n"
                "}\n"
        )
        file(WRITE ${SHADER_HEADER} ${SHADER_HEADER_OUTPUT})
        set_target_properties(Shaders PROPERTIES DEPENDS ${SHADER_HEADER})

        add_library(${SHADER_NAME}_obj OBJECT ${SHADER_OUTPUT}.c.h)
        target_compile_options(${SHADER_NAME}_obj PRIVATE
                $<$<C_COMPILER_ID:MSVC>:/FIstdint.h>
                $<$<C_COMPILER_ID:GNU,Clang>:-include stdint.h>
        )

        target_link_libraries(${ADD_SHADER_TARGET_NAME} PRIVATE ${SHADER_NAME}_obj)
    endforeach ()

    target_include_directories(${ADD_SHADER_TARGET_NAME} PRIVATE ${INCLUDE_DIR})
endfunction()