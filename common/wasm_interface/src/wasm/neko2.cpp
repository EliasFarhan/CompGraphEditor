
#include "wasm/neko2.h"

#include "engine/scene.h"

namespace neko2 {

namespace draw {

void bind(DrawCommand *drawCommand)
{
  drawCommand->drawCommand->Bind();
}
void set_float(DrawCommand *drawCommand, std::string_view name, float value)
{
  drawCommand->drawCommand->SetFloat(name, value);
}
void draw(DrawCommand *drawCommand)
{
  auto* scene = core::GetCurrentScene();
  drawCommand->drawCommand->PreDrawBind();
  scene->Draw(*drawCommand->drawCommand);
}
}

}