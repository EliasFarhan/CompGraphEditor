//
// Created by unite on 06.11.2025.
//

#include "novus/mesh.h"

namespace novus
{
std::unique_ptr<internal::VertexInputStateT>
GenerateVertexInputState(EnableNormal enableNormal, EnableTangent enableTangent, EnableBitangent enableBiTangent)
{
    auto vertexInputState = std::make_unique<internal::VertexInputStateT>();
    vertexInputState->vertex_attributes.push_back(
        {.location = 0, .buffer_slot = 0, .format = internal::VertexElementFormat_FLOAT3, .offset = 0});
    vertexInputState->vertex_attributes.push_back({.location = 1,
                                                   .buffer_slot = 0,
                                                   .format = internal::VertexElementFormat_FLOAT2,
                                                   .offset = sizeof(float) * 3});
    if (enableNormal == EnableNormal::Yes)
    {
        vertexInputState->vertex_attributes.push_back({.location = 2,
                                                       .buffer_slot = 0,
                                                       .format = internal::VertexElementFormat_FLOAT3,
                                                       .offset = sizeof(float) * 5});
    }
    if (enableTangent == EnableTangent::Yes)
    {
        vertexInputState->vertex_attributes.push_back({.location = 1,
                                                       .buffer_slot = 0,
                                                       .format = internal::VertexElementFormat_FLOAT3,
                                                       .offset = sizeof(float) * 8});
    }
    if (enableBiTangent == EnableBitangent::Yes)
    {
        vertexInputState->vertex_attributes.push_back({.location = 1,
                                                       .buffer_slot = 0,
                                                       .format = internal::VertexElementFormat_FLOAT3,
                                                       .offset = sizeof(float) * 11});
    }
    vertexInputState->vertex_buffer_descriptions.push_back({.slot = 0,
                                                            .pitch = sizeof(core::Vertex),
                                                            .input_rate = internal::VertexInputRate_VERTEX,
                                                            .instance_step_rate = 1});

    return vertexInputState;
}
} // namespace novus
