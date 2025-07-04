#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "renderer/command.h"

namespace neko2 {

namespace draw {
struct DrawCommand {
  core::DrawCommand* drawCommand;
};

extern void bind(DrawCommand* drawCommand);
extern void set_float(DrawCommand* drawCommand, std::string_view name, float value);
extern void draw(DrawCommand* drawCommand);
}
}