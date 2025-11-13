function(compile_shaders_to_vk main_folder exe_name)
    if(MSVC)
        if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64" OR ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "ARM64")
            set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslc.exe")
            set(SPIRV_CROSS "$ENV{VULKAN_SDK}/Bin/spirv-cross.exe")
        endif()
    elseif(UNIX)
        set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/bin/glslc")
        set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/bin/spirv-cross")
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
            "${main_folder}/data/*.glsl"
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
                COMMAND ${GLSL_VALIDATOR} ${GLSL} -o ${GLSL_OUTPUT}.spv --target-env=vulkan1.0
                COMMAND ${SPIRV_CROSS} ${GLSL_OUTPUT}.spv --output ${GLSL_OUTPUT}.json --reflect --dump-resources

                DEPENDS ${GLSL})
        list(APPEND GLSL_OUTPUT_FILES ${GLSL_OUTPUT})
    endforeach(GLSL)

    set(SHADER_TARGET "${exe_name}_ShadersCheck")
    MESSAGE(${SHADER_TARGET})
    add_custom_target(
            "${SHADER_TARGET}"
            DEPENDS ${GLSL_OUTPUT_FILES}
    )
    add_dependencies(${exe_name} "${SHADER_TARGET}")
endfunction()

function(compile_shaders_to_dx main_folder exe_name)
    if(MSVC)
        if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64" OR ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "ARM64")
            set(DXC "$ENV{VULKAN_SDK}/Bin/dxc.exe")
            set(SPIRV_CROSS "$ENV{VULKAN_SDK}/Bin/spirv-cross.exe")
        endif()
    elseif(UNIX)
        return()
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
            "${main_folder}/data/*.glsl"
    )
    file(GLOB_RECURSE VERT_SOURCE_FILES
            "${main_folder}/data/*.vert"
    )
    file(GLOB_RECURSE FRAG_SOURCE_FILES
            "${main_folder}/data/*.frag"
    )
    file(GLOB_RECURSE COMP_SOURCE_FILES
            "${main_folder}/data/*.comp"
    )
    foreach(GLSL ${VERT_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        get_filename_component(PATH_NAME ${GLSL} DIRECTORY)
        get_filename_component(EXTENSION ${GLSL} EXT)

        file(RELATIVE_PATH RELATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data/shaders" ${GLSL})
        get_filename_component(RELATIVE_PATH ${RELATIVE_PATH} DIRECTORY)
        file(RELATIVE_PATH PATH_NAME "${main_folder}" ${PATH_NAME})
        #MESSAGE("GLSL PATH: ${PATH_NAME} NAME: ${FILE_NAME}")
        set(GLSL_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
        #MESSAGE("GLSL OUT PATH: ${GLSL_OUTPUT}")
        add_custom_command(
                OUTPUT ${GLSL_OUTPUT}.hlsl
                COMMAND ${SPIRV_CROSS} ${GLSL_OUTPUT}.spv --hlsl --shader-model 60 --stage vert --entry "main" --output ${GLSL_OUTPUT}.hlsl
                COMMAND ${DXC} -T vs_6_0 -E "main" -Fo ${GLSL_OUTPUT}.dxil ${GLSL_OUTPUT}.hlsl
                #COMMAND $<TARGET_FILE:shadercross> ${GLSL_OUTPUT}.spv -d DXIL -e "main" -o ${GLSL_OUTPUT}.cso
                DEPENDS ${GLSL})
        list(APPEND GLSL_OUTPUT_FILES ${GLSL_OUTPUT}.hlsl)
    endforeach(GLSL)

    foreach(GLSL ${FRAG_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        get_filename_component(PATH_NAME ${GLSL} DIRECTORY)
        get_filename_component(EXTENSION ${GLSL} EXT)

        file(RELATIVE_PATH RELATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data/shaders" ${GLSL})
        get_filename_component(RELATIVE_PATH ${RELATIVE_PATH} DIRECTORY)
        file(RELATIVE_PATH PATH_NAME "${main_folder}" ${PATH_NAME})
        #MESSAGE("GLSL PATH: ${PATH_NAME} NAME: ${FILE_NAME}")
        set(GLSL_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
        #MESSAGE("GLSL OUT PATH: ${GLSL_OUTPUT}")
        add_custom_command(
                OUTPUT ${GLSL_OUTPUT}.hlsl
                COMMAND ${SPIRV_CROSS} ${GLSL_OUTPUT}.spv --hlsl --shader-model 60 --stage frag --entry "main" --output ${GLSL_OUTPUT}.hlsl
                COMMAND ${DXC} -T ps_6_0 -E "main" -Fo ${GLSL_OUTPUT}.dxil ${GLSL_OUTPUT}.hlsl
                #COMMAND $<TARGET_FILE:shadercross> ${GLSL_OUTPUT}.spv -d DXIL -e "main" -o ${GLSL_OUTPUT}.cso
                DEPENDS ${GLSL})
        list(APPEND GLSL_OUTPUT_FILES ${GLSL_OUTPUT}.hlsl)
    endforeach(GLSL)

    foreach(GLSL ${COMP_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        get_filename_component(PATH_NAME ${GLSL} DIRECTORY)
        get_filename_component(EXTENSION ${GLSL} EXT)

        file(RELATIVE_PATH RELATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data/shaders" ${GLSL})
        get_filename_component(RELATIVE_PATH ${RELATIVE_PATH} DIRECTORY)
        file(RELATIVE_PATH PATH_NAME "${main_folder}" ${PATH_NAME})
        #MESSAGE("GLSL PATH: ${PATH_NAME} NAME: ${FILE_NAME}")
        set(GLSL_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
        #MESSAGE("GLSL OUT PATH: ${GLSL_OUTPUT}")
        add_custom_command(
                OUTPUT ${GLSL_OUTPUT}.hlsl
                COMMAND ${SPIRV_CROSS} ${GLSL_OUTPUT}.spv --hlsl --shader-model 60 --stage comp --entry "main" --output ${GLSL_OUTPUT}.hlsl
                COMMAND ${DXC} -T cs_6_0 -E "main" -Fo ${GLSL_OUTPUT}.dxil ${GLSL_OUTPUT}.hlsl
                DEPENDS ${GLSL})
        list(APPEND GLSL_OUTPUT_FILES ${GLSL_OUTPUT}.hlsl)
    endforeach(GLSL)

    set(SHADER_TARGET "${exe_name}_DxShadersCheck")
    MESSAGE(${SHADER_TARGET})
    add_custom_target(
            "${SHADER_TARGET}"
            DEPENDS ${GLSL_OUTPUT_FILES}
    )
    add_dependencies("${SHADER_TARGET}" "${exe_name}_ShadersCheck")
    add_dependencies(${exe_name} "${SHADER_TARGET}")
endfunction()


function(compile_shaders_to_msl main_folder exe_name)
    if(MSVC)
        if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64" OR ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "ARM64")
            set(SPIRV_CROSS "$ENV{VULKAN_SDK}/Bin/spirv-cross.exe")
        endif()
    elseif(UNIX)
        return()
    endif()

    file(GLOB_RECURSE GLSL_SOURCE_FILES
            "${main_folder}/data/*.frag"
            "${main_folder}/data/*.vert"
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
        MESSAGE("GLSL PATH: ${PATH_NAME} NAME: ${FILE_NAME}")
        set(GLSL_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
        MESSAGE("GLSL OUT PATH: ${GLSL_OUTPUT}")
        add_custom_command(
                OUTPUT ${GLSL_OUTPUT}.msl
                COMMAND ${SPIRV_CROSS} ${GLSL_OUTPUT}.spv --msl --entry "main" --output ${GLSL_OUTPUT}.msl
                DEPENDS ${GLSL})
        list(APPEND GLSL_OUTPUT_FILES ${GLSL_OUTPUT}.spv)
    endforeach(GLSL)

    set(SHADER_TARGET "${exe_name}_MslShadersCheck")
    MESSAGE(${SHADER_TARGET})
    add_custom_target(
            "${SHADER_TARGET}"
            DEPENDS ${GLSL_OUTPUT_FILES}
    )
    add_dependencies("${SHADER_TARGET}" "${exe_name}_ShadersCheck")
    add_dependencies(${exe_name} "${SHADER_TARGET}")
endfunction()