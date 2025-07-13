#include "wasm_interface.h"
#include <wasm3_cpp.h>

#include "engine/filesystem.h"
#include "wasm/neko2.h"

namespace core
{
void LinkFunctions(wasm3::wasm_module &module)
{
    module.link_optional("*", "bind_draw_command", bind_draw_command);
    module.link_optional("*", "set_mat4_local", set_mat4_local);
    module.link_optional("*", "set_mat4_host", set_mat4_host);
    module.link_optional("*", "set_float", set_float);
    module.link_optional("*", "draw", draw);
    module.link_optional("*", "get_aspect", get_aspect);
    module.link_optional("*", "get_scene_camera", get_scene_camera);
    module.link_optional("*", "fill_camera_view", fill_camera_view);
    module.link_optional("*", "fill_camera_projection", fill_camera_projection);
    module.link_optional("*", "get_subpass_index", get_subpass_index);
    module.link_optional("*", "fill_camera_position", fill_camera_position);
    module.link_optional("*", "name_equals", name_equals);
    module.link_optional("*", "get_name", get_name);
    module.link_optional("*", "set_vec3_local", set_vec3_local);
    module.link_optional("*", "set_vec3_host", set_vec3_host);

}

WasmSystem::WasmSystem(wasm3::wasm_environment& env,
        wasm3::wasm_runtime& runtime,
        const FileBuffer& wasmFile,
    std::string_view moduleName) :
    module_(env.parse_module(wasmFile.data, wasmFile.size))
{
    runtime.load(module_);
    LinkFunctions(module_);
    const auto beginFuncName = std::format("{}_begin", moduleName);
    const auto updateFuncName = std::format("{}_update", moduleName);
    const auto endFuncName = std::format("{}_end", moduleName);

    try
    {
        begin_fn = runtime.find_function(beginFuncName.data());
    }
    catch (const wasm3::error& e)
    {
        LogWarning(std::format("Could not load {} in {}", beginFuncName, moduleName));
    }
    try
    {
        update_fn = runtime.find_function(updateFuncName.data());
    }
    catch (const wasm3::error& e)
    {
        LogWarning(std::format("Could not load {} in {}", updateFuncName, moduleName));
    }
    try
    {
        end_fn = runtime.find_function(endFuncName.data());
    }
    catch (const wasm3::error& e)
    {
        LogWarning(std::format("Could not load {} in {}", endFuncName, moduleName));
    }

    const auto drawFuncName = std::format("{}_draw", moduleName);
    try
    {
        draw_fn = runtime.find_function(drawFuncName.data());
    }
    catch (const wasm3::error& e)
    {
        LogDebug(std::format("No {} function, error: {}", drawFuncName, e.what()));
    }
}

void WasmSystem::Begin()
{
    if (begin_fn.has_value())
    {
        begin_fn->call();
    }
}

void WasmSystem::Update(float dt)
{
    if (update_fn.has_value())
    {
        update_fn->call(dt);
    }
}

void WasmSystem::End()
{
    if (end_fn.has_value())
    {
        end_fn->call();
    }
}

void WasmSystem::Draw(DrawCommand* sceneDrawCommand)
{
    if (draw_fn.has_value())
    {
        draw_fn->call(reinterpret_cast<int64_t>(sceneDrawCommand));
    }
}

void WasmSystem::OnKeyDown(SDL_Keycode keycode) {}

void WasmSystem::OnKeyUp(SDL_Keycode keycode) {}

void WasmSystem::OnMouseMotion(glm::vec2 mouseMotion) {}

void WasmSystem::OnMouseButtonDown(int mouseButton) {}

void WasmSystem::OnMouseButtonUp(int mouseButton) {}

void WasmSystem::Dispatch(ComputeCommand* command) {}

void WasmSystem::Trace(Command* command) {}

WasmManager::WasmManager() { ScriptLoaderLocator::provide(this); }
void WasmManager::Begin()
{
    MinimalScriptLoader::Begin();
}
Script* WasmManager::LoadScript(std::string_view path, std::string_view module, std::string_view className)
{
    auto* nativeScript = MinimalScriptLoader::LoadScript(path, module, className);
    if (nativeScript != nullptr)
    {
        nativeScript->Begin();
        return nativeScript;
    }
    std::unique_ptr<WasmSystem> wasmSystem = nullptr;
    try
    {
        auto wasmFile = LoadFile(path);
        if (wasmFile.data == nullptr)
        {
            LogError(std::format("Failed to open wasm file {}", path));
            return nullptr;
        }

        wasmSystem = std::make_unique<WasmSystem>(env,runtime, wasmFile, module);
    }
    catch (wasm3::error& e)
    {
        LogError(std::format("Could not load {}", path));
        return nullptr;
    }
    wasmSystems_.push_back(std::move(wasmSystem));
    return wasmSystems_.back().get();
}
} // namespace core
