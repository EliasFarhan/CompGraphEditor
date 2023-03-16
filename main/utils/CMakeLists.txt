add_executable(model_util model_util/model_util.cpp)
target_link_libraries(model_util assimp::assimp argh nlohmann_json::nlohmann_json fmt::fmt)

add_executable(cubemap_generator cubemap_generator/cubemap_generator.cpp)
target_link_libraries(cubemap_generator CoreProto fmt::fmt argh)