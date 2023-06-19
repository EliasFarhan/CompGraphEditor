#pragma once
#include "vk/common.h"
#include "proto/renderer.pb.h"
#include "renderer/framebuffer.h"

namespace vk
{

constexpr VkFormat GetFramebufferFormat(core::pb::RenderTarget_Format format, 
    core::pb::RenderTarget_FormatSize formatSize, 
    core::pb::RenderTarget_Type type)
{
    switch (format)
    {
    case core::pb::RenderTarget_Format_RED:
    {
        switch(formatSize)
        {
        case core::pb::RenderTarget_FormatSize_SIZE_8:
        {
            switch(type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: return VK_FORMAT_R8_UINT;
            case core::pb::RenderTarget_Type_INT: return VK_FORMAT_R8_SINT;
            case core::pb::RenderTarget_Type_FLOAT: return VK_FORMAT_R8_SRGB;
            default: ;
            }
            break;
        }
        case core::pb::RenderTarget_FormatSize_SIZE_16:
        {
            switch(type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: return VK_FORMAT_R16_UINT;
            case core::pb::RenderTarget_Type_INT: return VK_FORMAT_R16_SINT;
            case core::pb::RenderTarget_Type_FLOAT: return VK_FORMAT_R16_SFLOAT;
            default: ;
            }
            break;
        }
        case core::pb::RenderTarget_FormatSize_SIZE_32:
        {
            switch(type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: return VK_FORMAT_R32_UINT;
            case core::pb::RenderTarget_Type_INT: return VK_FORMAT_R32_SINT;
            case core::pb::RenderTarget_Type_FLOAT: return VK_FORMAT_R32_SFLOAT;
            default: break;
            }
            break;
        }
        default: break;
        }
        break;
    }
    case core::pb::RenderTarget_Format_RG: 
    {
        switch(formatSize)
        {
        case core::pb::RenderTarget_FormatSize_SIZE_8:
        {
            switch(type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: return VK_FORMAT_R8G8_UINT;
            case core::pb::RenderTarget_Type_INT: return VK_FORMAT_R8G8_SINT;
            case core::pb::RenderTarget_Type_FLOAT: return VK_FORMAT_R8G8_SRGB;
            default: break;
            }
            break;
        }
        case core::pb::RenderTarget_FormatSize_SIZE_16:
        {
            switch (type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: return VK_FORMAT_R16G16_UINT;
            case core::pb::RenderTarget_Type_INT: return VK_FORMAT_R16G16_SINT;
            case core::pb::RenderTarget_Type_FLOAT: return VK_FORMAT_R16G16_SFLOAT;
            default:;
            }
            break;
        }
        case core::pb::RenderTarget_FormatSize_SIZE_32: 
        {
            switch (type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: return VK_FORMAT_R32G32_UINT;
            case core::pb::RenderTarget_Type_INT: return VK_FORMAT_R32G32_SINT;
            case core::pb::RenderTarget_Type_FLOAT: return VK_FORMAT_R32G32_SFLOAT;
            default:;
            }
            break;
        }
        default: ;
        }
        break;
    }
    case core::pb::RenderTarget_Format_RGBA:
    case core::pb::RenderTarget_Format_RGB:
    {
        switch(formatSize)
        {
        case core::pb::RenderTarget_FormatSize_SIZE_8:  
        {
            switch(type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: return VK_FORMAT_R8G8B8A8_UINT;
            case core::pb::RenderTarget_Type_INT: return VK_FORMAT_R8G8B8A8_SINT;
            case core::pb::RenderTarget_Type_FLOAT: return VK_FORMAT_R8G8B8A8_SRGB;
            default: break;
            }
            break;
        }
        case core::pb::RenderTarget_FormatSize_SIZE_16:
        {
            switch (type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: return VK_FORMAT_R16G16B16A16_UINT;
            case core::pb::RenderTarget_Type_INT: return VK_FORMAT_R16G16B16A16_SINT;
            case core::pb::RenderTarget_Type_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
            default: break;
            }
            break;
        }
        case core::pb::RenderTarget_FormatSize_SIZE_32:
        {
            switch (type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: return VK_FORMAT_R32G32B32A32_UINT;
            case core::pb::RenderTarget_Type_INT: return VK_FORMAT_R32G32B32A32_SINT;
            case core::pb::RenderTarget_Type_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
            default: break;
            }
            break;
        }
        default: ;
        }
        break;
    }
    case core::pb::RenderTarget_Format_DEPTH_COMP:
    {
        switch(formatSize)
        {
        case core::pb::RenderTarget_FormatSize_SIZE_8:
        {
            switch(type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: break;
            case core::pb::RenderTarget_Type_INT: break;
            case core::pb::RenderTarget_Type_FLOAT: break;
            default: break;
            }
            break;
        }
        case core::pb::RenderTarget_FormatSize_SIZE_16:
        {
            switch(type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: return VK_FORMAT_D16_UNORM;
            case core::pb::RenderTarget_Type_INT: break;
            case core::pb::RenderTarget_Type_FLOAT: break;
            default: break;
            }
            break;
        }
        case core::pb::RenderTarget_FormatSize_SIZE_24:
        {
            break;
        }
        case core::pb::RenderTarget_FormatSize_SIZE_32:
        {
            switch (type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: break;
            case core::pb::RenderTarget_Type_INT: break;
            case core::pb::RenderTarget_Type_FLOAT: return VK_FORMAT_D32_SFLOAT;
            default: break;
            }
            break;
        }
        default: ;
        }
        break;
    }
    case core::pb::RenderTarget_Format_DEPTH_STENCIL:
    {
        switch (formatSize)
        {
        case core::pb::RenderTarget_FormatSize_SIZE_8:
        {
            break;
        }
        case core::pb::RenderTarget_FormatSize_SIZE_16:
        {
            switch(type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: return VK_FORMAT_D16_UNORM_S8_UINT;
            case core::pb::RenderTarget_Type_INT: break;
            case core::pb::RenderTarget_Type_FLOAT: break;
            default: ;
            }
            break;
        }
        case core::pb::RenderTarget_FormatSize_SIZE_24:
        {
            switch(type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: return VK_FORMAT_D24_UNORM_S8_UINT;
            case core::pb::RenderTarget_Type_INT: break;
            case core::pb::RenderTarget_Type_FLOAT: break;
            default: ;
            }
            break;
        }
        case core::pb::RenderTarget_FormatSize_SIZE_32:
        {
            switch(type)
            {
            case core::pb::RenderTarget_Type_UNSIGNED: break;
            case core::pb::RenderTarget_Type_INT: break;
            case core::pb::RenderTarget_Type_FLOAT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
            default: ;
            }
            break;
        }
        default: ;
        }
        break;
    }
    case core::pb::RenderTarget_Format_LUMINANCE_ALPHA: break;
    case core::pb::RenderTarget_Format_ALPHA: break;
    default: break;
    }
    return VK_FORMAT_MAX_ENUM;
}

struct RenderTarget
{
    std::string name;
    Image image;
    VkImageView imageView;
    VkFormat format;
    VkSampler sampler = VK_NULL_HANDLE;
};

/**
 * @brief Framebuffer is a class that represents a GL-like abstraction, not the vkFramebuffer.
 * It contains the render target (Image and Image View)
 */
class Framebuffer : public core::Framebuffer
{
public:
    void Load(const core::pb::FrameBuffer& framebufferPb) override;
    void Destroy();
    RenderTarget& GetRenderTarget(std::string_view attachmentName);
    std::string_view GetName() const { return name; }
    std::unique_ptr<core::Image> GetImage(std::string_view attachmentName) override;
private:
    std::vector<RenderTarget> targets;
    std::string name;
};
}
