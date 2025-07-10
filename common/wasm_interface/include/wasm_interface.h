#pragma once

#include "engine/filesystem.h"
#include "engine/script.h"
#include <wasm3_cpp.h>

namespace core
{
class WasmSystem final : public Script
{
public:
    explicit WasmSystem(wasm3::wasm_environment& env,
        wasm3::wasm_runtime& runtime,
        const FileBuffer& wasmFile,
        std::string_view moduleName);
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void Draw(DrawCommand* sceneDrawCommand) override;
    void OnKeyDown(SDL_Keycode keycode) override;
    void OnKeyUp(SDL_Keycode keycode) override;
    void OnMouseMotion(glm::vec2 mouseMotion) override;
    void OnMouseButtonDown(int mouseButton) override;
    void OnMouseButtonUp(int mouseButton) override;
    void Dispatch(ComputeCommand* command) override;
    void Trace(Command* command) override;

private:
    wasm3::wasm_module module_;
    std::optional<wasm3::wasm_function> begin_fn;
    std::optional<wasm3::wasm_function> update_fn;
    std::optional<wasm3::wasm_function> end_fn;
    std::optional<wasm3::wasm_function> draw_fn;
    std::optional<wasm3::wasm_function> on_key_down_fn;
    std::optional<wasm3::wasm_function> on_key_up_fn;
    std::optional<wasm3::wasm_function> on_mouse_motion_fn;
    std::optional<wasm3::wasm_function> on_mouse_button_down_fn;
    std::optional<wasm3::wasm_function> on_mouse_button_up_fn;
    std::optional<wasm3::wasm_function> dispatch_fn;
    std::optional<wasm3::wasm_function> trace_fn;
};

class WasmManager final : public MinimalScriptLoader
{
public:
    WasmManager();
    void Begin() override;
    Script* LoadScript(std::string_view path, std::string_view module, std::string_view className) override;

private:
    wasm3::wasm_environment env;
    wasm3::wasm_runtime runtime = env.new_runtime(1024*1024);
    std::vector<std::unique_ptr<WasmSystem>> wasmSystems_;
};
} // namespace core
