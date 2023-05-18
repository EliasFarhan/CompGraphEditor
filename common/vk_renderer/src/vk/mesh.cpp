#include "vk/mesh.h"

#include "vk/engine.h"

namespace vk
{

VertexBuffer CreateVertexBufferFromMesh(const core::Mesh& mesh)
{
    auto& engine = GetEngine();
    const auto bufferSize = mesh.vertices.size()*sizeof(core::Vertex);
    const Buffer stagingVertexBuffer = CreateBuffer(bufferSize,
                                                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    const auto& allocator = GetAllocator();
    void* data;
    vmaMapMemory(allocator, stagingVertexBuffer.allocation, &data);
    std::memcpy(data, mesh.vertices.data(), bufferSize);
    vmaUnmapMemory(allocator, stagingVertexBuffer.allocation);
    auto vertexFlag = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if(HasRaytracing())
    {

        vertexFlag |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    }
    const auto vertexBuffer = CreateBuffer(bufferSize, vertexFlag,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CopyBuffer(stagingVertexBuffer, vertexBuffer, bufferSize);
    vmaDestroyBuffer(allocator, stagingVertexBuffer.buffer, stagingVertexBuffer.allocation);
    //Upload index buffer to GPU
    const Buffer stagingIndexBuffer = CreateBuffer(mesh.indices.size() * sizeof(unsigned),
                                                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vmaMapMemory(allocator, stagingIndexBuffer.allocation, &data);
    std::memcpy(data, mesh.indices.data(), mesh.indices.size() * sizeof(unsigned));
    vmaUnmapMemory(allocator, stagingIndexBuffer.allocation);

    auto indexFlag = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if(HasRaytracing())
    {
        indexFlag |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    }

    const auto indexBuffer = CreateBuffer(mesh.indices.size()*sizeof(unsigned),
        indexFlag,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CopyBuffer(stagingIndexBuffer, indexBuffer, mesh.indices.size() * sizeof(unsigned));
    vmaDestroyBuffer(allocator, stagingIndexBuffer.buffer, stagingIndexBuffer.allocation);

    return {vertexBuffer, mesh.vertices.size(), indexBuffer, mesh.indices.size()};
}
} // namespace vk
