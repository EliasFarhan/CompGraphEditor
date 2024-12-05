#include "lua_interface.h"

namespace core
{
Script* LuaManager::LoadScript(std::string_view path, std::string_view module, std::string_view className)
{
	return MinimalScriptLoader::LoadScript(path, module, className);
}

void LuaManager::Begin()
{
	luaState_ = std::make_unique<sol::state>();
}

void LuaManager::End()
{
	luaState_ = nullptr;
}

void LuaManager::ImportScript(std::string_view module, std::string_view className,
	std::unique_ptr<Script> script)
{
}
}