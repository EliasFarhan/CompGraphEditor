#include "lua_interface.h"

#include "engine/scene.h"
#include "renderer/framebuffer.h"
#include "renderer/texture.h"
#include "renderer/pipeline.h"
#include "renderer/command.h"
#include "renderer/buffer.h"

namespace core
{

void ImplementBinding(sol::state& lua)
{
	auto module = lua["neko2"].get_or_create<sol::table>();
	module.new_usertype<core::Script>("System",
		"on_begin", &core::Script::Begin,
		"on_update", &core::Script::Update,
		"on_end", &core::Script::End,
		"on_draw", &core::Script::Draw,
		"on_dispatch", &core::Script::Dispatch,
		"on_key_up", &core::Script::OnKeyUp,
		"on_key_down", &core::Script::OnKeyDown,
		"on_mouse_motion", &core::Script::OnMouseMotion);
	module.new_usertype<core::Image>("Image");
	module.new_usertype<core::ComputeCommand>("ComputeCommand",
		"set_float",
		&core::ComputeCommand::SetFloat,
		"set_int",
		&core::ComputeCommand::SetInt,
		"set_bool",
		&core::ComputeCommand::SetBool,
		"set_vec2",
		&core::ComputeCommand::SetVec2,
		"set_vec3",
		&core::ComputeCommand::SetVec3,
		"set_vec4",
		&core::ComputeCommand::SetVec4,
		"set_mat3",
		&core::ComputeCommand::SetMat3,
		"set_mat4",
		&core::ComputeCommand::SetMat4,
		"bind",
		&core::ComputeCommand::Bind,
		"dispatch",
		[](core::ComputeCommand& command, int x, int y, int z)
		{
		  auto* scene = core::GetCurrentScene();
		  scene->Dispatch(command, x, y, z);
		},
		"set_image",
		[](core::ComputeCommand& command, core::Image* image, int bindingPoint, core::Image::AccessType access)
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
		"set_mat4", &core::DrawCommand::SetMat4,
		"draw", [](core::DrawCommand& drawCommand)
		{
		  auto* scene = core::GetCurrentScene();
		  drawCommand.PreDrawBind();
		  scene->Draw(drawCommand);
		},
		"draw_instanced", [](core::DrawCommand& drawCommand, int instance)
		{
		  auto* scene = core::GetCurrentScene();
		  drawCommand.PreDrawBind();
		  scene->Draw(drawCommand, instance);
		},
		"bind", &core::DrawCommand::Bind,
		"get_material", [](core::DrawCommand& drawCommand)
		{
		  auto* scene = core::GetCurrentScene();
		  return scene->GetMaterial(drawCommand.GetMaterialIndex());
		},
		"get_name", &core::DrawCommand::GetName,
		"get_mesh_name", [](core::DrawCommand& drawCommand)
		{
		  auto* scene = core::GetCurrentScene();
		  return scene->GetMeshName(drawCommand.GetMeshIndex());
		},
		"get_subpass_index", &core::DrawCommand::GetSubpassIndex,
		"model_matrix", &core::DrawCommand::modelTransformMatrix
	);
	module.new_usertype<core::SceneSubPass>("SubPass",
		"get_draw_command", &core::SceneSubPass::GetDrawCommand,
		"get_draw_command_count", &core::SceneSubPass::GetDrawCommandCount,
		"get_framebuffer", &core::SceneSubPass::GetFramebuffer
	);
	module.new_usertype<core::Framebuffer>("Framebuffer",
		"get_image", &core::Framebuffer::GetImage);

	module.new_usertype<core::Scene>("Scene",
		"get_pipeline", &core::Scene::GetPipeline,
		"get_material", &core::Scene::GetMaterial,
		"get_subpass", &core::Scene::GetSubpass,
		"get_framebuffer", &core::Scene::GetFramebuffer,
		"get_camera", &core::Scene::GetCamera,
		"get_subpass_count", &core::Scene::GetSubpassCount,
		"get_pipeline_count", &core::Scene::GetPipelineCount,
		"get_material_count", &core::Scene::GetMaterialCount);

	module.new_usertype<glm::vec2>("Vec2",
		sol::constructors<glm::vec2(),
						  glm::vec2(float),
						  glm::vec2(float, float),
						  glm::vec2(const glm::vec3&),
						  glm::vec2(const glm::vec4&)>(),
		"x", &glm::vec2::x,
		"y", &glm::vec2::y,
		"dot", [](glm::vec2 v1, glm::vec2 v2)
		{ return glm::dot(v1, v2); },
		"normalize", [](const glm::vec2& v)
		{ return glm::normalize(v); },
		"length", [](glm::vec2 v)
		{ return glm::length(v); }
	);

	module.set_function("get_scene", []{
	  return core::GetCurrentScene();
	});
	module.set_function("get_window_size", []
	{
	  return glm::vec2{ core::GetWindowSize() };
	});
	module.set_function("get_aspect", []
	{
	  const auto windowSize = core::GetWindowSize();
	  return static_cast<float>(windowSize.x)/static_cast<float>(windowSize.y);
	});
	module.new_usertype<core::SceneMaterial>("Material",
		"get_pipeline", &core::SceneMaterial::GetPipeline,
		"get_name", &core::SceneMaterial::GetName);
	module.new_usertype<core::Camera>("Camera",
		"position", &core::Camera::position,
		"direction", &core::Camera::direction,
		"up", &core::Camera::up,
		"near", &core::Camera::near,
		"far", &core::Camera::far,
		"fov", &core::Camera::fovY,
		"aspect", &core::Camera::aspect,
		"orthographic_size", &core::Camera::orthographicHalfHeight,
		"projection_type", &core::Camera::projectionType,
		"get_view", &core::Camera::GetView,
		"get_projection", &core::Camera::GetProjection);

	module.new_usertype<core::Pipeline>("Pipeline",
		"get_name", &core::Pipeline::GetPipelineName);

	module.new_usertype<core::ModelTransformMatrix>("ModelTransformMatrix",
		"get_translate", &core::ModelTransformMatrix::GetTranslate,
		"get_scale", &core::ModelTransformMatrix::GetScale,
		"get_rotation", &core::ModelTransformMatrix::GetRotation,
		"transform", &core::ModelTransformMatrix::GetModelTransformMatrix);

	module.new_usertype<core::CameraSystem>("CameraSystem",
		"camera", &core::CameraSystem::camera);

	module.set_function("get_camera_system", &core::GetCameraSystem);
	module.set_function("get_buffer", [](std::string_view bufferName)
	{
	  auto& bufferManager = core::GetCurrentScene()->GetBufferManager();
	  return bufferManager.GetBuffer(bufferName);
	});
}

Script* LuaManager::LoadScript(std::string_view path, std::string_view module, std::string_view className)
{
	auto* nativeScript = MinimalScriptLoader::LoadScript(path, module, className);
	if(nativeScript != nullptr)
	{
		nativeScript->Begin();
		return nativeScript;
	}
	if(!FileExists(path))
	{
		LogError(fmt::format("Could not find script file at path: {}", path));
		return nullptr;
	}
	auto fileBuffer = LoadFile(path);
	auto loadResult = luaState_->load_buffer(reinterpret_cast<const char*>(fileBuffer.data), fileBuffer.size);
	if(!loadResult.valid())
	{
		sol::error err = loadResult;
		LogError(fmt::format("Error loading lua script: {}", err.what()));
		return nullptr;
	}
	luaState_->script(fmt::format("instance = {}.new()", className));
	luaState_->script("instance:begin()");

	return (*luaState_)["instance"];
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

}