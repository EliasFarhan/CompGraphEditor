
function(copydata main_folder exe_name)
    file(GLOB_RECURSE DATA_FILES
            "${main_folder}/data/*.json"
            "${main_folder}/data/*.png"
            "${main_folder}/data/*.jpg"
            "${main_folder}/data/*.cube"
            "${main_folder}/data/*.bmp"
            "${main_folder}/data/*.hdr"
            "${main_folder}/data/*.obj"
            "${main_folder}/data/*.mtl"
            "${main_folder}/data/*.bin"
            "${main_folder}/data/*.rpass"
            "${main_folder}/data/*.py"
            )
    foreach(DATA ${DATA_FILES})
        get_filename_component(FILE_NAME ${DATA} NAME)
        get_filename_component(PATH_NAME ${DATA} DIRECTORY)
        get_filename_component(EXTENSION ${DATA} EXT)
        file(RELATIVE_PATH PATH_NAME "${main_folder}" ${PATH_NAME})
        #MESSAGE("Data PATH: ${PATH_NAME} NAME: ${FILE_NAME} EXTENSION: ${EXTENSION}")
        set(DATA_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
		
		if("${EXTENSION}" STREQUAL ".py")
			source_group("Script Files\\${RELATIVE_PATH}" FILES "${DATA}")
		endif()
        #MESSAGE("Data OUT PATH: ${DATA_OUTPUT}")
        add_custom_command(
                OUTPUT ${DATA_OUTPUT}
                COMMAND ${CMAKE_COMMAND} -E copy
                ${main_folder}/${PATH_NAME}/${FILE_NAME}
                ${DATA_OUTPUT}
                DEPENDS ${DATA})
        list(APPEND Data_OUTPUT_FILES ${DATA_OUTPUT})
    endforeach(DATA)

    set(DATA_TARGET "${exe_name}_Data")
    MESSAGE(${DATA_TARGET})

    add_custom_target(
            "${exe_name}_DATA"
            DEPENDS ${DATA_FILES} ${Data_OUTPUT_FILES}
    )
    add_dependencies(${exe_name} "${exe_name}_DATA")
endfunction()

function(checkshaders main_folder exe_name)
    if(MSVC)
        if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
            set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
        else()
            set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe")
        endif()
    elseif(UNIX)
        set(GLSL_VALIDATOR "glslangValidator")
    endif()

    file(GLOB_RECURSE GLSL_SOURCE_FILES
            "${main_folder}/data/*.frag"
            "${main_folder}/data/*.vert"
            "${main_folder}/data/*.tesc"
            "${main_folder}/data/*.tese"
            "${main_folder}/data/*.geom"
            "${main_folder}/data/*.comp"
            )
    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        get_filename_component(PATH_NAME ${GLSL} DIRECTORY)
        get_filename_component(EXTENSION ${GLSL} EXT)

        file(RELATIVE_PATH RELATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data/shaders" ${GLSL})
        get_filename_component(RELATIVE_PATH ${RELATIVE_PATH} DIRECTORY)

        source_group("Shader Files\\${RELATIVE_PATH}" FILES "${GLSL}")
        file(RELATIVE_PATH PATH_NAME "${main_folder}" ${PATH_NAME})
        #MESSAGE("GLSL PATH: ${PATH_NAME} NAME: ${FILE_NAME}")
        set(GLSL_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
        #MESSAGE("GLSL OUT PATH: ${GLSL_OUTPUT}")
        add_custom_command(
                OUTPUT ${GLSL_OUTPUT}
                COMMAND ${CMAKE_COMMAND} -E copy
                ${main_folder}/${PATH_NAME}/${FILE_NAME}
                "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}"
                COMMAND ${GLSL_VALIDATOR}  ${GLSL}
                DEPENDS ${GLSL})
        list(APPEND GLSL_OUTPUT_FILES ${GLSL_OUTPUT})
    endforeach(GLSL)

    set(SHADER_TARGET "${exe_name}_ShadersCheck")
    MESSAGE(${SHADER_TARGET})
    add_custom_target(
            "${exe_name}_ShadersCheck"
            DEPENDS ${GLSL_OUTPUT_FILES}
    )
    add_dependencies(${exe_name} "${exe_name}_ShadersCheck")
endfunction()


function(checkvkshaders main_folder exe_name)
    if(MSVC)
        if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
            set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslc.exe")
        else()
            set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslc.exe")
        endif()
    elseif(UNIX)
        set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/bin/glslc")
    endif()

    file(GLOB_RECURSE GLSL_SOURCE_FILES
            "${main_folder}/data/*.frag"
            "${main_folder}/data/*.vert"
            "${main_folder}/data/*.tesc"
            "${main_folder}/data/*.tese"
            "${main_folder}/data/*.geom"
            "${main_folder}/data/*.comp"
            "${main_folder}/data/*.rchit"
            "${main_folder}/data/*.rmiss"
            "${main_folder}/data/*.rgen"
            "${main_folder}/data/*.rint"
            "${main_folder}/data/*.rahit"
            "${main_folder}/data/*.rcall"
            )
    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        get_filename_component(PATH_NAME ${GLSL} DIRECTORY)
        get_filename_component(EXTENSION ${GLSL} EXT)

        file(RELATIVE_PATH RELATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data/shaders" ${GLSL})
        get_filename_component(RELATIVE_PATH ${RELATIVE_PATH} DIRECTORY)

        source_group("Shader Files\\${RELATIVE_PATH}" FILES "${GLSL}")
        file(RELATIVE_PATH PATH_NAME "${main_folder}" ${PATH_NAME})
        #MESSAGE("GLSL PATH: ${PATH_NAME} NAME: ${FILE_NAME}")
        set(GLSL_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
        #MESSAGE("GLSL OUT PATH: ${GLSL_OUTPUT}")
        add_custom_command(
                OUTPUT ${GLSL_OUTPUT}
                COMMAND ${CMAKE_COMMAND} -E copy
                ${main_folder}/${PATH_NAME}/${FILE_NAME}
                ${GLSL_OUTPUT}
                COMMAND ${GLSL_VALIDATOR} ${GLSL} -o ${GLSL_OUTPUT}.spv --target-env=vulkan1.2
                DEPENDS ${GLSL})
        list(APPEND GLSL_OUTPUT_FILES ${GLSL_OUTPUT})
    endforeach(GLSL)

    set(SHADER_TARGET "${exe_name}_ShadersCheck")
    MESSAGE(${SHADER_TARGET})
    add_custom_target(
            "${exe_name}_ShadersCheck"
            DEPENDS ${GLSL_OUTPUT_FILES}
    )
    add_dependencies(${exe_name} "${exe_name}_ShadersCheck")
endfunction()