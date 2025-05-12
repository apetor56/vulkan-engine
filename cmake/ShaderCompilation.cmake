function(compile_shaders SHADER_LIST COMPILED_SHADERS)
    find_program(GLSLC glslc
        HINTS "$ENV{VULKAN_SDK}/Bin"
        REQUIRED
    )
    if(NOT GLSLC)
        message(FATAL_ERROR "glslc not found. Make sure the Vulkan SDK is installed and VULKAN_SDK env is set.")

    endif()
    set(SHADER_BINARIES)
    foreach(SHADER_PATH ${SHADER_LIST})
        get_filename_component(SHADER_NAME ${SHADER_PATH} NAME)
        set(SHADER_BINARY "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv")
        add_custom_command(
            OUTPUT ${SHADER_BINARY}
            DEPENDS ${SHADER_PATH}
            COMMAND ${GLSLC} ${SHADER_PATH} -o ${SHADER_BINARY}
            COMMENT "Compiling ${SHADER_PATH}"
            VERBATIM
        )
        list(APPEND SHADER_BINARIES ${SHADER_BINARY})
    endforeach()

    set(${COMPILED_SHADERS} ${SHADER_BINARIES} PARENT_SCOPE)
endfunction()
