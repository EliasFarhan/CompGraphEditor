#pragma once
#include <string_view>

namespace editor
{
void GeneratePreComputeBrdfLUT();
void GenerateIrradianceMap(const core::Path& path);
void GeneratePreFilterEnvMap(const core::Path& path);
} // namespace gpr5300
