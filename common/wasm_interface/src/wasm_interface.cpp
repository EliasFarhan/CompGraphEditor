#include "wasm_interface.h"
#include <wasm3_cpp.h>

#include "engine/filesystem.h"

namespace core
{
WasmSystem::WasmSystem(wasm3::wasm_runtime& runtime)
	: begin_fn(runtime.find_function("begin")),
	  update_fn(runtime.find_function("update")), end_fn(runtime.find_function("end"))
{
}

void WasmSystem::Begin()
{
	begin_fn.call();
}

void WasmSystem::Update(float dt)
{
	update_fn.call(dt);
}

void WasmSystem::End()
{
	end_fn.call();
}

void WasmSystem::Draw(DrawCommand* sceneDrawCommand)
{
}

void WasmSystem::OnKeyDown(SDL_Keycode keycode)
{
}

void WasmSystem::OnKeyUp(SDL_Keycode keycode)
{
}

void WasmSystem::OnMouseMotion(glm::vec2 mouseMotion)
{
}

void WasmSystem::OnMouseButtonDown(int mouseButton)
{
}

void WasmSystem::OnMouseButtonUp(int mouseButton)
{
}

void WasmSystem::Dispatch(ComputeCommand* command)
{
}

void WasmSystem::Trace(Command* command)
{
}

Script* WasmManager::LoadScript(std::string_view path, std::string_view module,
	std::string_view className)
{
	auto* nativeScript = MinimalScriptLoader::LoadScript(path, module, className);
	if (nativeScript != nullptr)
	{
		nativeScript->Begin();
		return nativeScript;
	}
	try
	{
		auto wasmFile = LoadFile(path);
		if (wasmFile.data == nullptr)
		{
			LogError(std::format("Failed to open wasm file {}", path));
			return nullptr;
		}
		wasm3::wasm_module mod = env.parse_module(wasmFile.data, wasmFile.size);
		runtime.load(mod);

		wasmSystem_ = std::make_unique<WasmSystem>(runtime);
	}
	catch (wasm3::error& e)
	{
		LogError(std::format("Could not load {}", path));
		return nullptr;
	}

	return wasmSystem_.get();
}
} // namespace core
