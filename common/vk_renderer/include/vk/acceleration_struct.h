#pragma once

#include "vk/common.h"
#include "proto/renderer.pb.h"

namespace vk
{


/**
 * \brief AccelerationStrucutre is a class that describes a Bottom Level Acceleration Structure
 */
class BottomAccelerationStructure
{
public:
    bool Create(const core::pb::TopLevelAccelerationStructure_AccelerationStructure& accelerationStruct);
    void Destroy() const;
    std::uint64_t GetDeviceAddress() const;

private:
    AccelerationStructure blas_;
};

class TopLevelAccelerationStructure
{
public:
    bool Create(const core::pb::TopLevelAccelerationStructure& accelerationStructure);
    void Destroy() const;
    const VkAccelerationStructureKHR& GetHandle() const { return tlas_.handle; }
private:
    std::vector<BottomAccelerationStructure> bottomAccelerationStructures_;
    AccelerationStructure tlas_;
};
} // namespace vk
