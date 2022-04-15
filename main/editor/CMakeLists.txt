
file(GLOB_RECURSE EDITOR_SRC src/*.cpp include/*.h)
add_executable(editor ${EDITOR_SRC})
target_include_directories(editor PRIVATE include/)
target_link_libraries(editor PRIVATE Common)

set_target_properties(editor PROPERTIES UNITY_BUILD ON)