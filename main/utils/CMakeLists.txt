add_executable(model_util model_util/model_util.cpp)
target_link_libraries(model_util assimp::assimp argh nlohmann_json::nlohmann_json fmt::fmt)
set_target_properties (model_util PROPERTIES FOLDER Main/Utils)

add_executable(cubemap_generator cubemap_generator/cubemap_generator.cpp)
target_link_libraries(cubemap_generator CoreProto fmt::fmt argh)
set_target_properties (cubemap_generator PROPERTIES FOLDER Main/Utils)

add_executable(vk_shader_analyze vk_shader_analyze/vk_shader_analyze.cpp)
target_link_libraries(vk_shader_analyze argh nlohmann_json::nlohmann_json CommonVK spirv-cross-core spirv-cross-reflect)
set_target_properties (vk_shader_analyze PROPERTIES FOLDER Main/Utils)
 