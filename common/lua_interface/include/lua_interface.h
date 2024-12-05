#pragma once
#include "engine/script.h"

#include <sol/sol.hpp>
#include <memory>

namespace core
{

class LuaManager : public MinimalScriptLoader
{
public:
	Script* LoadScript(std::string_view path, std::string_view module, std::string_view className) override;

	void Begin() override;

	void End() override;

	void ImportScript(std::string_view module, std::string_view className, std::unique_ptr<Script> script) override;
private:
	std::unique_ptr<sol::state> luaState_;
};


}
