MESSAGE(VK_COMMON)

file(GLOB_RECURSE COMMON_VK_FILES src/*.cpp include/*.h)
add_library(CommonVK STATIC ${COMMON_VK_FILES})
target_include_directories(CommonVK PUBLIC include/)
target_link_libraries(CommonVK PUBLIC Core)

set_target_properties (CommonVK PROPERTIES FOLDER Core)