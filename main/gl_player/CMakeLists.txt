file(GLOB_RECURSE PLAYER_SRC src/*.cpp include/*.h)
find_package(PhysFS REQUIRED)
add_executable(player ${PLAYER_SRC} ${SCRIPT_FILES})
target_include_directories(player PRIVATE include/ ${PHYSFS_INCLUDE_DIR})
target_link_libraries(player PRIVATE CommonGL ${PHYSFS_LIBRARY})

set_target_properties(player PROPERTIES UNITY_BUILD ON)