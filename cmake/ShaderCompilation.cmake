function(compile_shaders SHADER_LIST COMPILED_SHADERS)
    set(SHADER_BINARIES)
    foreach(SHADER ${SHADER_LIST})
        set(SHADER_BINARY ${SHADER}.spv)
        add_custom_command(
            OUTPUT ${SHADER_BINARY}
            DEPENDS ${SHADER}
            COMMAND glslc ${SHADER} -o ${SHADER_BINARY}
            COMMENT "Compiling ${SHADER}"
            VERBATIM
        )
        list(APPEND SHADER_BINARIES ${SHADER_BINARY})
    endforeach()

    set(${COMPILED_SHADERS} ${SHADER_BINARIES} PARENT_SCOPE)
endfunction()
