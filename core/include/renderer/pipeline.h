#pragma once

namespace gpr5300
{

class Shader
{

};

class Pipeline
{
public:
    virtual ~Pipeline() = default;
    virtual void Bind() = 0;

};
} // namespace gpr5300
