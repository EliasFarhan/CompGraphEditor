
find_package(sol2 CONFIG REQUIRED)
find_package(Lua REQUIRED)

add_library(CommonLua STATIC include/lua_interface.h src/lua_interface.cpp)
target_include_directories(CommonLua PUBLIC include/)
target_include_directories(CommonLua PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(CommonLua PUBLIC Core sol2 ${LUA_LIBRARIES} )
set_target_properties (CommonLua PROPERTIES FOLDER Core)