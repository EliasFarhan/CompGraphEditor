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
    bool Create(const core::pb::RaytracingCommand_AccelerationStructure& accelerationStruct);
private:
    AccelerationStructure blas_;
};
} // namespace vk
