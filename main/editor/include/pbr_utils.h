#pragma once

namespace gpr5300
{
void GeneratePreComputeBrdfLUT();
void GenerateIrradianceMap(std::string_view path);
void GeneratePreFilterEnvMap(std::string_view path);
} // namespace gpr5300
