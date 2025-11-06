//
// Created by unite on 06.11.2025.
//

#ifndef NEKO2_MESH_H
#define NEKO2_MESH_H
#include <generated/internal_renderer_generated.h>


#include "renderer/buffer.h"
#include "renderer/mesh.h"
#include "utils/sdl_fb_impl.h"

namespace novus
{

class VertexInputBuffer : core::VertexInputBuffer
{
public:
    void CreateFromMesh(const core::Mesh& mesh) override;
    void Bind() override;
    void Destroy() override;
};

enum class EnableNormal
{
    Yes,
    No
};
enum class EnableTangent
{
    Yes,
    No
};
enum class EnableBitangent
{
    Yes,
    No
};
std::unique_ptr<internal::VertexInputStateT> GenerateVertexInputState(EnableNormal enableNormal=EnableNormal::No,
    EnableTangent enableTangent = EnableTangent::No,
    EnableBitangent enableBiTangent= EnableBitangent::No);
}

#endif // NEKO2_MESH_H
