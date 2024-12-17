#include "lua_interface.h"

int main()
{
  core::LuaManager manager;
  manager.Begin();
  auto* script = manager.LoadScript(
    "data/lua_test_system.lua",
    "data.lua_test_system",
    "LuaTestSystem");
  if (script == nullptr) {
    manager.End();
    return EXIT_FAILURE;
  }
  script->Begin();
  script->Update(0.1f);
  script->End();

  manager.End();
  return EXIT_SUCCESS;
}