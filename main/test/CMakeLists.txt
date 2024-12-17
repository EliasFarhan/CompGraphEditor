
add_executable(lua_test lua_test.cpp)
target_link_libraries(lua_test CommonLua)
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/data)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)
endif ()
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/lua_test_system.lua DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data/)