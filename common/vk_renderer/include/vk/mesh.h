#pragma once

#include "common.h"
#include "renderer/mesh.h"
#include "renderer/model.h"
#include "vk/mesh.h"

namespace vk
{
class Mesh : public core::Mesh
{
public:
    [[nodiscard]] std::string_view GetName() const override { return ""; }
};

struct VertexBuffer
{
    Buffer vertexBuffer;
    std::size_t verticesCount;
    Buffer indexBuffer;
    std::size_t indicesCount;
};

VertexBuffer CreateVertexBufferFromMesh(const core::refactor::Mesh& mesh);

}
