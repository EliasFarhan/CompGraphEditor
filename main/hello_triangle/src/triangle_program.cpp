#include "triangle_program.h"

namespace gpr5300
{
void HelloTriangleProgram::Begin()
{
    shaders_[0].LoadShader("data/shader/triangle.vert", Shader::ShaderType::VERTEX);
    shaders_[1].LoadShader("data/shader/triangle.frag", Shader::ShaderType::FRAGMENT);
    
}

void HelloTriangleProgram::Update(float dt)
{
}

void HelloTriangleProgram::End()
{
}
} // namespace gpr5300
