#pragma once
#include "engine/script.h"

namespace core
{
class WasmSystem final : public Script {
public:
  WasmSystem();
  void Begin() override;
  void Update(float dt) override;
  void End() override;
  void Draw(DrawCommand *sceneDrawCommand) override;
  void OnKeyDown(SDL_Keycode keycode) override;
  void OnKeyUp(SDL_Keycode keycode) override;
  void OnMouseMotion(glm::vec2 mouseMotion) override;
  void OnMouseButtonDown(int mouseButton) override;
  void OnMouseButtonUp(int mouseButton) override;
  void Dispatch(ComputeCommand *command) override;
  void Trace(Command *command) override;
private:

};
}