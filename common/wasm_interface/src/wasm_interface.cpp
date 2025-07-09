#include "wasm_interface.h"
#include <wasm3_cpp.h>

namespace core
{

WasmSystem::WasmSystem(wasm3::wasm_runtime &runtime)
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
void WasmSystem::End() {
  end_fn.call();
}
void WasmSystem::Draw(DrawCommand *sceneDrawCommand)
{

}
void WasmSystem::OnKeyDown(SDL_Keycode keycode) {

}
void WasmSystem::OnKeyUp(SDL_Keycode keycode) {

}

void WasmSystem::OnMouseMotion(glm::vec2 mouseMotion) {

}

void WasmSystem::OnMouseButtonDown(int mouseButton) {

}

void WasmSystem::OnMouseButtonUp(int mouseButton) {

}

void WasmSystem::Dispatch(ComputeCommand *command) {

}

void WasmSystem::Trace(Command *command) {}

Script *WasmManager::LoadScript(std::string_view path, std::string_view module,
                                std::string_view className) {
  auto* nativeScript = MinimalScriptLoader::LoadScript(path, module, className);
  if (nativeScript != nullptr) {
    nativeScript->Begin();
    return nativeScript;
  }

}
} // namespace core