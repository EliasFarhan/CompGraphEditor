#pragma once
#include <engine/filesystem.h>
#include <string_view>

namespace novus::editor
{
void GeneratePreComputeBrdfLUT();
void GenerateIrradianceMap(std::string_view path);
void GeneratePreFilterEnvMap(std::string_view path);
} // namespace gpr5300
