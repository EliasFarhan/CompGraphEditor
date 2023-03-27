#pragma once

#include <vk_mem_alloc.h>
#include <glm/ext/matrix_float4x4.hpp>

namespace vk
{

struct Buffer
{
    VkBuffer buffer{};
    VmaAllocation allocation{};
};

struct Image
{
    VkImage image{};
    VmaAllocation allocation{};
};

// Holds data for a ray tracing scratch buffer that is used as a temporary storage
struct RayTracingScratchBuffer
{
    std::uint64_t deviceAddress = 0;
    Buffer buffer;
};

// Ray tracing acceleration structure
struct AccelerationStructure
{
    VkAccelerationStructureKHR handle{};
    std::uint64_t deviceAddress = 0;
    Buffer buffer;
};

/**
 * \brief Simplifed structure used to combine sampler and image view to load directly in the descriptor sets
 */
struct Sampler
{
    VkSampler sampler;
    VkImageView imageView;
};

struct CameraData
{
    glm::mat4 view;
    glm::mat4 projection;
};

bool CheckError(VkResult result);
}
