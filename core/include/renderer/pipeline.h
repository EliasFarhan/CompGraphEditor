#pragma once

namespace core
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
} // namespace core
