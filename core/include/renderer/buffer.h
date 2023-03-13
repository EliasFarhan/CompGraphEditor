#pragma once

#include "renderer/model.h"

namespace core
{

class VertexBuffer
{
public:
    virtual ~VertexBuffer() = default;
    virtual void CreateFromMesh(const refactor::Mesh& mesh) = 0;
    virtual void Bind() = 0;
    virtual void Destroy() = 0;
};

} // namespace core