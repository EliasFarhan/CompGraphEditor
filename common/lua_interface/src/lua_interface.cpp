#include "lua_interface.h"

#include "engine/scene.h"
#include "renderer/framebuffer.h"
#include "renderer/texture.h"
#include "renderer/command.h"

namespace core
{

void ImplementBinding(sol::state& lua)
{
	auto module = lua["neko2"].get_or_create<sol::table>();
	module.new_usertype<core::Image>("Image");
	module.new_usertype<core::ComputeCommand>("ComputeCommand",
		"set_float", &core::ComputeCommand::SetFloat,
		"set_int", &core::ComputeCommand::SetInt,
		"set_bool", &core::ComputeCommand::SetBool,
		"set_vec2", &core::ComputeCommand::SetVec2,
		"set_vec3", &core::ComputeCommand::SetVec3,
		"set_vec4", &core::ComputeCommand::SetVec4,
		"set_mat3", &core::ComputeCommand::SetMat3,
		"set_mat4", &core::ComputeCommand::SetMat4,
		"bind", &core::ComputeCommand::Bind,
		"dispatch", [](core::ComputeCommand& command, int x, int y, int z)
		    {
			auto* scene = core::GetCurrentScene();
			scene->Dispatch(command, x, y, z);
		    },
		"set_image", [](core::ComputeCommand& command, core::Image* image, int bindingPoint, core::Image::AccessType access)
		    {
			if (image)
			{
			    image->BindImage(bindingPoint, access);
			}
			else
			{
			    LogError("Image is null");
			}
		    });
	module.new_usertype<core::DrawCommand>("DrawCommand",
		"set_float", &core::DrawCommand::SetFloat,
		"set_int", &core::DrawCommand::SetInt,
		"set_vec2", &core::DrawCommand::SetVec2,
		"set_vec3", &core::DrawCommand::SetVec3,
		"set_vec4", &core::DrawCommand::SetVec4,
		"set_mat3", &core::DrawCommand::SetMat3,
		"set_mat4", &core::DrawCommand::SetMat4
		);
}

Script* LuaManager::LoadScript(std::string_view path, std::string_view module, std::string_view className)
{
	return MinimalScriptLoader::LoadScript(path, module, className);
}

void LuaManager::Begin()
{
	luaState_ = std::make_unique<sol::state>();
	luaState_->open_libraries();
	ImplementBinding(*luaState_);
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