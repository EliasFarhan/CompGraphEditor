#pragma once
#include <generated/renderer_generated.h>


namespace core
{

class Image
{
public:
    enum class AccessType
    {
        READ_ONLY,
        WRITE_ONLY,
        READ_WRITE
    };
    virtual void BindImage(int bindPoint, AccessType access) = 0;
};

class Framebuffer
{
public:
    virtual void Load(const novus::renderer::FramebufferT& framebufferPb) = 0;
    virtual std::unique_ptr<Image> GetImage(std::string_view attachmentName) = 0;
};
} // namespace core
