function(compile_shaders SHADER_LIST COMPILED_SHADERS)
    set(SHADER_BINARIES)
    foreach(SHADER_PATH ${SHADER_LIST})
        get_filename_component(SHADER_NAME ${SHADER_PATH} NAME)
        set(SHADER_BINARY "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv")
        add_custom_command(
            OUTPUT ${SHADER_BINARY}
            DEPENDS ${SHADER_PATH}
            COMMAND glslc ${SHADER_PATH} -o ${SHADER_BINARY}
            COMMENT "Compiling ${SHADER_PATH}"
            VERBATIM
        )
        list(APPEND SHADER_BINARIES ${SHADER_BINARY})
    endforeach()

    set(${COMPILED_SHADERS} ${SHADER_BINARIES} PARENT_SCOPE)
endfunction()
