#include "wasm_interface.h"
#include <wasm3_cpp.h>

#include "engine/filesystem.h"
#include "wasm/neko2.h"

namespace core
{

void LinkFunctions(wasm3::wasm_module &module)
{
    module.link("*", "bind_draw_command", bind_draw_command);
    module.link("*", "set_float", set_float);
    module.link("*", "draw", draw);
}

WasmSystem::WasmSystem(wasm3::wasm_runtime& runtime, std::string_view moduleName, std::string_view beginFuncName,
                       std::string_view updateFuncName, std::string_view endFuncName) :
    begin_fn(runtime.find_function(beginFuncName.data())), update_fn(runtime.find_function(updateFuncName.data())),
    end_fn(runtime.find_function(endFuncName.data()))
{
    try
    {
        auto drawFuncName = std::format("{}_draw", moduleName);
        draw_fn = runtime.find_function(drawFuncName.data());
    }
    catch (const wasm3::error& e)
    {
    }
}

void WasmSystem::Begin() { begin_fn.call(); }

void WasmSystem::Update(float dt) { update_fn.call(dt); }

void WasmSystem::End() { end_fn.call(); }

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
        wasm3::wasm_module mod = env.parse_module(wasmFile.data, wasmFile.size);
        runtime.load(mod);
        LinkFunctions(mod);

        const auto beginFuncName = std::format("{}_begin", module);
        const auto endFuncName = std::format("{}_end", module);
        const auto updateFuncName = std::format("{}_update", module);
        wasmSystem = std::make_unique<WasmSystem>(runtime, module, beginFuncName, updateFuncName, endFuncName);
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
