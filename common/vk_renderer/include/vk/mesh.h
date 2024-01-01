#pragma once

#include "vk/common.h"
#include "renderer/mesh.h"
#include "renderer/model.h"

namespace vk
{

struct VertexInputBuffer
{
    Buffer vertexBuffer;
    std::size_t verticesCount;
    Buffer indexBuffer;
    std::size_t indicesCount;
};

VertexInputBuffer CreateVertexBufferFromMesh(const core::Mesh& mesh);

}
