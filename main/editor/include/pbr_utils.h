#pragma once
#include <string_view>

namespace editor
{
void GeneratePreComputeBrdfLUT();
void GenerateIrradianceMap(std::string_view path);
void GeneratePreFilterEnvMap(std::string_view path);
} // namespace gpr5300
