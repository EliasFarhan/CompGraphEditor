find_package(PhysFS REQUIRED)

add_library(PhysFilesystem STATIC src/phys_filesystem.cpp include/phys_filesystem.h)
target_include_directories(PhysFilesystem PUBLIC include/ ${PHYSFS_INCLUDE_DIR})
target_link_libraries(PhysFilesystem PUBLIC Core ${PHYSFS_LIBRARY})

set_target_properties (PhysFilesystem PROPERTIES FOLDER Core)