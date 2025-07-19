
function(compilescript main_folder exe_name)
    if(MSVC)
        set(EMCC "em++.bat")
    elseif(UNIX)
        set(EMCC "em++")
    endif()
    file(GLOB_RECURSE DATA_FILES
            "${main_folder}/data/*.cpp"
    )
    foreach(DATA ${DATA_FILES})
        get_filename_component(FILE_NAME ${DATA} NAME_WE)
        get_filename_component(PATH_NAME ${DATA} DIRECTORY)
        get_filename_component(EXTENSION ${DATA} EXT)
        file(RELATIVE_PATH PATH_NAME "${main_folder}" ${PATH_NAME})
        #MESSAGE("Data PATH: ${PATH_NAME} NAME: ${FILE_NAME} EXTENSION: ${EXTENSION}")
        set(DATA_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}.wasm")

        if("${EXTENSION}" STREQUAL ".cpp")
            source_group("Script Files\\${RELATIVE_PATH}" FILES "${DATA}")
        endif()

        #MESSAGE("Data OUT PATH: ${DATA_OUTPUT}")
        add_custom_command(
                OUTPUT ${DATA_OUTPUT}
                COMMAND ${EMCC} ${DATA} -o ${DATA_OUTPUT} -I${CMAKE_SOURCE_DIR}/common/wasm_interface/include/
                -I${CMAKE_SOURCE_DIR}/core/include -I${CMAKE_BINARY_DIR}/core/ -I${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include
                -std=c++20 -s STANDALONE_WASM -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s PURE_WASI -msimd128
                DEPENDS ${DATA})
        list(APPEND Data_OUTPUT_FILES ${DATA_OUTPUT})
    endforeach(DATA)

    set(DATA_TARGET "${exe_name}_Wasm")
    MESSAGE(${DATA_TARGET})

    add_custom_target(
            "${exe_name}_Wasm"
            DEPENDS ${DATA_FILES} ${Data_OUTPUT_FILES}
    )
    add_dependencies(${exe_name} "${exe_name}_Wasm")
endfunction()
