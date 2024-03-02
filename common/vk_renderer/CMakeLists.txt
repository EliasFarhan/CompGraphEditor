MESSAGE(VK_COMMON)

find_package(volk CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

file(GLOB_RECURSE COMMON_VK_FILES src/*.cpp include/*.h)
add_library(CommonVK STATIC ${COMMON_VK_FILES})
target_include_directories(CommonVK PUBLIC include/)
target_link_libraries(CommonVK PUBLIC Core volk::volk GPUOpen::VulkanMemoryAllocator)
target_compile_definitions(CommonVK PUBLIC VK_NO_PROTOTYPES)

set_target_properties (CommonVK PROPERTIES FOLDER Core)
target_precompile_headers(CommonVK REUSE_FROM Core)