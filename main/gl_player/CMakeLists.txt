file(GLOB_RECURSE PLAYER_SRC src/*.cpp include/*.h)
find_package(PhysFS REQUIRED)
add_executable(gl_player ${PLAYER_SRC} ${SCRIPT_FILES})
target_include_directories(gl_player PRIVATE include/ ${PHYSFS_INCLUDE_DIR})
target_link_libraries(gl_player PRIVATE CommonGL ${PHYSFS_LIBRARY})

set_target_properties(gl_player PROPERTIES UNITY_BUILD ON)
set_target_properties (gl_player PROPERTIES FOLDER Main)