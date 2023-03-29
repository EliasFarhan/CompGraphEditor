#pragma once

namespace core
{
class Framebuffer
{
public:
    virtual void Load(const core::pb::FrameBuffer& framebufferPb) = 0;
};
} // namespace core
