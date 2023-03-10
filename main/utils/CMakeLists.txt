add_executable(model_util model_util/model_util.cpp)
target_link_libraries(model_util assimp::assimp argh nlohmann_json::nlohmann_json fmt::fmt)