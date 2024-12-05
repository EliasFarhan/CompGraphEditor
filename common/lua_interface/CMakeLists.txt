
find_package(sol2 CONFIG REQUIRED)

add_library(CommonLua STATIC include/lua_interface.h src/lua_interface.cpp)
target_include_directories(CommonLua PUBLIC include/)
target_link_libraries(CommonLua PUBLIC Core sol2)
set_target_properties (CommonLua PROPERTIES FOLDER Core)