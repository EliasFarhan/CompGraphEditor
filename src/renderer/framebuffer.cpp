#include "renderer/framebuffer.h"

namespace gpr5300
{
void Framebuffer::Load(const pb::FrameBuffer& framebufferPb)
{
    for(const auto& colorAttachmentInfo : framebufferPb.color_attachments())
    {
        
    }
}

AttachmentType GetAttachmentType(const pb::RenderTarget& renderTargetInfo)
{
    GLint internalFormat = 0;
    GLenum format = 0;
    GLenum type = 0;
    GLint error = 0;
    switch (renderTargetInfo.type())
    {
    case pb::RenderTarget_Type_UNSIGNED:
        switch (renderTargetInfo.format_size())
        {
        case pb::RenderTarget_FormatSize_SIZE_8:
            type = GL_UNSIGNED_BYTE;
            switch (renderTargetInfo.format())
            {
            case pb::RenderTarget_Format_RED:
                internalFormat = GL_R8UI;
                format = GL_RED_INTEGER;
                break;
            case pb::RenderTarget_Format_RG:
                internalFormat = GL_RG8UI;
                format = GL_RG_INTEGER;
                break;
            case pb::RenderTarget_Format_RGB:
                internalFormat = GL_RGB8UI;
                format = GL_RGB_INTEGER;
                break;
            case pb::RenderTarget_Format_RGBA:
                internalFormat = GL_RGBA8UI;
                format = GL_RGBA_INTEGER;
                break;
            default: 
                error = 1;
                break;
            }
            break;
        case pb::RenderTarget_FormatSize_SIZE_16:
            type = GL_UNSIGNED_SHORT;
            switch (renderTargetInfo.format())
            {
            case pb::RenderTarget_Format_RED:
                internalFormat = GL_R16UI;
                format = GL_RED_INTEGER;
                break;
            case pb::RenderTarget_Format_RG:
                internalFormat = GL_RG16UI;
                format = GL_RG_INTEGER;
                break;
            case pb::RenderTarget_Format_RGB:
                internalFormat = GL_RGB16UI;
                format = GL_RGB_INTEGER;
                break;
            case pb::RenderTarget_Format_RGBA:
                internalFormat = GL_RGBA16UI;
                format = GL_RGBA_INTEGER;
                break;
            case pb::RenderTarget_Format_DEPTH_COMP:
                internalFormat = GL_DEPTH_COMPONENT16;
                format = GL_DEPTH_COMPONENT;
            default: 
                error = 1;
                break;
            }
            break;
        case pb::RenderTarget_FormatSize_SIZE_24:
            switch (renderTargetInfo.format())
            {
            case pb::RenderTarget_Format_DEPTH_COMP:
                type = GL_UNSIGNED_INT;
                internalFormat = GL_DEPTH_COMPONENT24;
                format = GL_DEPTH_COMPONENT;
                break;
            case pb::RenderTarget_Format_DEPTH_STENCIL:
                type = GL_UNSIGNED_INT;
                internalFormat = GL_DEPTH24_STENCIL8;
                format = GL_DEPTH_STENCIL;
                type = GL_UNSIGNED_INT_24_8;
                break;
            default:
                error = 1;
                break;
            }
            break;
        case pb::RenderTarget_FormatSize_SIZE_32:
            type = GL_UNSIGNED_INT;
            switch (renderTargetInfo.format())
            {
            case pb::RenderTarget_Format_RED:
                internalFormat = GL_R32UI;
                format = GL_RED_INTEGER;
                break;
            case pb::RenderTarget_Format_RG:
                internalFormat = GL_RG32UI;
                format = GL_RG_INTEGER;
                break;
            case pb::RenderTarget_Format_RGB:
                internalFormat = GL_RGB32UI;
                format = GL_RGB_INTEGER;
                break;
            case pb::RenderTarget_Format_RGBA:
                internalFormat = GL_RGBA32UI;
                format = GL_RGBA_INTEGER;
                break;
            default: 
                error = 1;
                break;
            }
            break;
        default: 
            error = 1;
            break;
        }
        break;
    case pb::RenderTarget_Type_INT:
        switch (renderTargetInfo.format_size())
        {
        case pb::RenderTarget_FormatSize_SIZE_8:
            type = GL_BYTE;
            switch(renderTargetInfo.format())
            {
            case pb::RenderTarget_Format_RED: 
                internalFormat = GL_R8I;
                format = GL_RED_INTEGER;
                break;
            case pb::RenderTarget_Format_RG: 
                internalFormat = GL_RG8I;
                format = GL_RG_INTEGER;
                break;
            case pb::RenderTarget_Format_RGB:
                internalFormat = GL_RGB8I;
                format = GL_RGB_INTEGER;
                break;
            case pb::RenderTarget_Format_RGBA:
                internalFormat = GL_RGBA8I;
                format = GL_RGBA_INTEGER;
                break;
            default: 
                error = 1;
                break;
            }
            break;
        case pb::RenderTarget_FormatSize_SIZE_16: 
            type = GL_SHORT;
            switch (renderTargetInfo.format())
            {
            case pb::RenderTarget_Format_RED:
                internalFormat = GL_R16I;
                format = GL_RED_INTEGER;
                break;
            case pb::RenderTarget_Format_RG:
                internalFormat = GL_RG16I;
                format = GL_RG_INTEGER;
                break;
            case pb::RenderTarget_Format_RGB:
                internalFormat = GL_RGB16I;
                format = GL_RGB_INTEGER;
                break;
            case pb::RenderTarget_Format_RGBA:
                internalFormat = GL_RGBA16I;
                format = GL_RGBA_INTEGER;
                break;
            default:
                error = 1;
                break;
            }
            break;
        case pb::RenderTarget_FormatSize_SIZE_32: 
            type = GL_INT;
            switch (renderTargetInfo.format())
            {
            case pb::RenderTarget_Format_RED:
                internalFormat = GL_R32I;
                format = GL_RED_INTEGER;
                break;
            case pb::RenderTarget_Format_RG:
                internalFormat = GL_RG32I;
                format = GL_RG_INTEGER;
                break;
            case pb::RenderTarget_Format_RGB:
                internalFormat = GL_RGB32I;
                format = GL_RGB_INTEGER;
                break;
            case pb::RenderTarget_Format_RGBA:
                internalFormat = GL_RGBA32I;
                format = GL_RGBA_INTEGER;
                break;
            default:
                error = 1;
                break;
            }
            break;
        default:
            error = 1;
            break;
        }
        break;
    case pb::RenderTarget_Type_FLOAT:
        switch (renderTargetInfo.format_size())
        {
        case pb::RenderTarget_FormatSize_SIZE_8: 
            type = GL_BYTE;
            switch (renderTargetInfo.format())
            {
            case pb::RenderTarget_Format_RED:
                internalFormat = GL_R8;
                format = GL_RED;
                break;
            case pb::RenderTarget_Format_RG:
                internalFormat = GL_R16;
                format = GL_RG;
                break;
            case pb::RenderTarget_Format_RGB:
                internalFormat = renderTargetInfo.snorm() ? GL_SRGB8 : GL_RGB8;
                format = GL_RGB;
                break;
            case pb::RenderTarget_Format_RGBA:
                internalFormat = renderTargetInfo.snorm() ? GL_SRGB8_ALPHA8 : GL_RGBA8;
                format = GL_RGBA;
                break;
            default:
                error = 1;
                break;
            }
            break;
        case pb::RenderTarget_FormatSize_SIZE_16:
            type = GL_FLOAT;
            switch (renderTargetInfo.format())
            {
            case pb::RenderTarget_Format_RED:
                internalFormat = GL_R16F;
                format = GL_RED;
                break;
            case pb::RenderTarget_Format_RG:
                internalFormat = GL_RG16F;
                format = GL_RG;
                break;
            case pb::RenderTarget_Format_RGB:
                internalFormat = GL_RGB16F;
                format = GL_RGB;
                break;
            case pb::RenderTarget_Format_RGBA:
                internalFormat = GL_RGBA16F;
                format = GL_RGBA;
                break;
            default:
                error = 1;
                break;
            }
            break;
        case pb::RenderTarget_FormatSize_SIZE_32: 
            type = GL_FLOAT;
            switch (renderTargetInfo.format())
            {
            case pb::RenderTarget_Format_RED:
                internalFormat = GL_R32F;
                format = GL_RED;
                break;
            case pb::RenderTarget_Format_RG:
                internalFormat = GL_RG32F;
                format = GL_RG;
                break;
            case pb::RenderTarget_Format_RGB:
                internalFormat = GL_RGB32F;
                format = GL_RGB;
                break;
            case pb::RenderTarget_Format_RGBA:
                internalFormat = GL_RGBA32F;
                format = GL_RGBA;
                break;
            case pb::RenderTarget_Format_DEPTH_COMP:
                internalFormat = GL_DEPTH_COMPONENT32F;
                format = GL_DEPTH_COMPONENT;
                break;
            case pb::RenderTarget_Format_DEPTH_STENCIL:
                internalFormat = GL_DEPTH32F_STENCIL8;
                format = GL_DEPTH_STENCIL;
                type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
                break;
            default:
                error = 1;
                break;
            }
            break;
        default:
            error = 1;
            break;
        }
        break;
    default: 
        error = 1;
        break;
    }
    return { internalFormat, format, type, error };
}
} // namespace gpr5300
