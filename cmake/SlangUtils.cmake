# Modified from https://thatonegamedev.com/cpp/cmake/how-to-compile-shaders-with-cmake/#graphics-programming-with-vulkan

function(add_shaders TARGET_NAME)
    set(SHADER_SOURCE_FILES ${ARGN}) # the rest of arguments to this function will be assigned as shader source files

    # Validate that source files have been passed
    list(LENGTH SHADER_SOURCE_FILES FILE_COUNT)
    if (FILE_COUNT EQUAL 0)
        message(FATAL_ERROR "Cannot create a shaders target without any source files")
    endif ()

    foreach (SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
        cmake_path(ABSOLUTE_PATH SHADER_SOURCE NORMALIZE)
        cmake_path(GET SHADER_SOURCE STEM SHADER_NAME)

        # Build command
        list(APPEND SHADER_COMMAND COMMAND)
        list(APPEND SHADER_COMMAND ${SLANG_EXECUTABLE})
        list(APPEND SHADER_COMMAND "-target")
        list(APPEND SHADER_COMMAND "spirv")
        list(APPEND SHADER_COMMAND "-fvk-use-entrypoint-name")
        list(APPEND SHADER_COMMAND "-o")
        list(APPEND SHADER_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.spv")
        list(APPEND SHADER_COMMAND "${SHADER_SOURCE}")

        add_custom_command(
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.spv"
                COMMENT "Compiling ${SHADER_NAME}.slang to SPIR-V"
                COMMAND ${SHADER_COMMAND}
                DEPENDS ${SHADER_SOURCE}
        )

        add_custom_target(${SHADER_NAME} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.spv)

        add_dependencies(${TARGET_NAME} ${SHADER_NAME})
    endforeach ()
endfunction()