#include "vk/mesh.h"

#include "vk/engine.h"

namespace vk
{

VertexBuffer CreateVertexBufferFromMesh(const core::Mesh& mesh)
{
    auto& engine = GetEngine();
    const auto bufferSize = mesh.vertices.size()*sizeof(core::Vertex);
    const Buffer stagingVertexBuffer = engine.CreateBuffer(bufferSize,
                                                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    const auto& allocator = GetAllocator();
    void* data;
    vmaMapMemory(allocator, stagingVertexBuffer.allocation, &data);
    std::memcpy(data, mesh.vertices.data(), bufferSize);
    vmaUnmapMemory(allocator, stagingVertexBuffer.allocation);
    const auto vertexBuffer = engine.CreateBuffer(bufferSize,
                                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    engine.CopyBuffer(stagingVertexBuffer, vertexBuffer, bufferSize);
    vmaDestroyBuffer(allocator, stagingVertexBuffer.buffer, stagingVertexBuffer.allocation);
    //Upload index buffer to GPU
    const Buffer stagingIndexBuffer = engine.CreateBuffer(mesh.indices.size() * sizeof(unsigned),
                                                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vmaMapMemory(allocator, stagingIndexBuffer.allocation, &data);
    std::memcpy(data, mesh.indices.data(), mesh.indices.size() * sizeof(unsigned));
    vmaUnmapMemory(allocator, stagingIndexBuffer.allocation);

    const auto indexBuffer = engine.CreateBuffer(mesh.indices.size()*sizeof(unsigned),
                                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    engine.CopyBuffer(stagingIndexBuffer, indexBuffer, mesh.indices.size() * sizeof(unsigned));
    vmaDestroyBuffer(allocator, stagingIndexBuffer.buffer, stagingIndexBuffer.allocation);

    return {vertexBuffer, mesh.vertices.size(), indexBuffer, mesh.indices.size()};
}
} // namespace vk
