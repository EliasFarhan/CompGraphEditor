#include "renderer/framebuffer.h"
#include "renderer/debug.h"
#include "engine/engine.h"
#include "utils/log.h"

#include <fmt/format.h>

namespace gpr5300
{
Framebuffer::~Framebuffer()
{
    if (name_ != 0)
    {
        LogWarning("Forgot to clear Framebuffer");
    }
}

void Framebuffer::Bind() const
{
    if (currentFramebuffer_ != name_)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, name_);
        currentFramebuffer_ = name_;
        glCheckError();
    }
}

void Framebuffer::Resize(glm::uvec2 windowSize)
{
    Bind();
    if(frameBufferPb_.has_depth_stencil_attachment())
    {
        const auto& depthStencilAttachmentInfo = frameBufferPb_.depth_stencil_attachment();
        const auto attachmentType = GetAttachmentType(depthStencilAttachmentInfo);
        bool stencil = attachmentType.format == GL_DEPTH_STENCIL;
        if(depthStencilAttachmentInfo.size_type() == pb::RenderTarget_Size_WINDOW_SIZE)
        {
            if (depthStencilAttachmentInfo.rbo())
            {
                if(depthStencilAttachment_ != 0)
                {
                    glDeleteRenderbuffers(1, &depthStencilAttachment_);
                    depthStencilAttachment_ = 0;
                }
                glGenRenderbuffers(1, &depthStencilAttachment_);
                glBindRenderbuffer(GL_RENDERBUFFER, depthStencilAttachment_);
                glRenderbufferStorage(GL_RENDERBUFFER, attachmentType.internalFormat,
                    windowSize.x,
                    windowSize.y);

                glFramebufferRenderbuffer(GL_FRAMEBUFFER, stencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthStencilAttachment_);
                glCheckError();
            }
            else
            {
                if (depthStencilAttachment_ != 0)
                {
                    glDeleteTextures(1, &depthStencilAttachment_);
                    depthStencilAttachment_ = 0;
                }
                glCreateTextures(GL_TEXTURE_2D, 1, &depthStencilAttachment_);
                glBindTexture(GL_TEXTURE_2D, depthStencilAttachment_);
                glTexImage2D(GL_TEXTURE_2D, 0, attachmentType.internalFormat,
                    windowSize.x,
                    windowSize.y,
                    0,
                    attachmentType.format,
                    attachmentType.type,
                    nullptr
                );
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glFramebufferTexture2D(GL_FRAMEBUFFER, stencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthStencilAttachment_, 0);

                const auto& name = depthStencilAttachmentInfo.name();
                if (!name.empty())
                {
                    textureMap_[name] = depthStencilAttachment_;
                }
                glCheckError();
            }
        }
    }
    if (frameBufferPb_.color_attachments_size() == 0)
    {
        constexpr GLenum v = GL_NONE;
        glDrawBuffers(1, &v);
        glReadBuffer(GL_NONE);
    }
    for (int i = 0; i < frameBufferPb_.color_attachments_size(); i++)
    {

        auto& colorAttachment = colorAttachments_[i];
        const auto& colorAttachmentInfo = frameBufferPb_.color_attachments(i);
        if (colorAttachmentInfo.size_type() == pb::RenderTarget_Size_FIXED_SIZE)
            continue;
        const auto attachmentType = GetAttachmentType(colorAttachmentInfo);
        if (colorAttachmentInfo.rbo())
        {
            if (colorAttachment != 0)
            {
                glDeleteRenderbuffers(1, &colorAttachment);
                colorAttachment = 0;
            }
            glGenRenderbuffers(1, &colorAttachment);
            glBindRenderbuffer(GL_RENDERBUFFER, colorAttachment);
            glRenderbufferStorage(GL_RENDERBUFFER, attachmentType.internalFormat,
                windowSize.x,
                windowSize.y);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, colorAttachment);
            glCheckError();
        }
        else
        {
            if (colorAttachment != 0)
            {
                glDeleteTextures(1, &colorAttachment);
                colorAttachment = 0;
            }
            glCreateTextures(GL_TEXTURE_2D, 1, &colorAttachment);
            glBindTexture(GL_TEXTURE_2D, colorAttachment);
            glTexImage2D(GL_TEXTURE_2D, 0, attachmentType.internalFormat,
                windowSize.x,
                windowSize.y,
                0,
                attachmentType.format,
                attachmentType.type,
                nullptr
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorAttachment, 0);

            const auto& name = colorAttachmentInfo.name();
            if (!name.empty())
            {
                textureMap_[name] = colorAttachment;
            }
            glCheckError();
        }
    }
    
}

void Framebuffer::Destroy()
{
    glDeleteFramebuffers(1, &name_);
    for(int i = 0; i < frameBufferPb_.color_attachments_size(); i++)
    {
        if(frameBufferPb_.color_attachments(i).rbo())
        {
            glDeleteRenderbuffers(1, &colorAttachments_[i]);
        }
        else
        {
            glDeleteTextures(1, &colorAttachments_[i]);
        }
    }
    if(depthStencilAttachment_ != 0)
    {
        if(frameBufferPb_.depth_stencil_attachment().rbo())
        {
            glDeleteRenderbuffers(1, &depthStencilAttachment_);
        }
        else
        {
            glDeleteTextures(1, &depthStencilAttachment_);
        }
    }
}

void Framebuffer::Unbind()
{
    if (currentFramebuffer_ != 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCheckError();
        currentFramebuffer_ = 0;
    }
}

void Framebuffer::Load(const pb::FrameBuffer& framebufferPb)
{
    const auto windowSize = Engine::GetInstance()->GetWindowSize();
    frameBufferPb_ = framebufferPb;
    framebufferName_ = framebufferPb.name();
    glCreateFramebuffers(1, &name_);
    Bind();
    colorAttachments_.resize(framebufferPb.color_attachments_size());
    for(int i = 0; i < framebufferPb.color_attachments_size(); i++)
    {
        auto& colorAttachment = colorAttachments_[i];
        const auto& colorAttachmentInfo = framebufferPb.color_attachments(i);
        const auto attachmentType = GetAttachmentType(colorAttachmentInfo);
        if (attachmentType.error != 0)
        {
            LogError(fmt::format("Could not get attachment type for color attachment {}", i));
        }
        if (colorAttachmentInfo.rbo())
        {

            glGenRenderbuffers(1, &colorAttachment);
            glBindRenderbuffer(GL_RENDERBUFFER, colorAttachment);
            glRenderbufferStorage(GL_RENDERBUFFER, attachmentType.internalFormat, 
                colorAttachmentInfo.size_type() == pb::RenderTarget_Size_WINDOW_SIZE ? windowSize.x : colorAttachmentInfo.target_size().x(),
                colorAttachmentInfo.size_type() == pb::RenderTarget_Size_WINDOW_SIZE ? windowSize.y : colorAttachmentInfo.target_size().y());

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, colorAttachment);
            glCheckError();
        }
        else
        {
            const auto target = colorAttachmentInfo.cubemap() ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
            glCreateTextures(target, 1, &colorAttachment);
            glBindTexture(target, colorAttachment);

            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            if(colorAttachmentInfo.cubemap())
            {
                glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                for(int face = 0; face < 6; face++)
                {
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, 0, attachmentType.internalFormat,
                        colorAttachmentInfo.size_type() == pb::RenderTarget_Size_WINDOW_SIZE ? windowSize.x : colorAttachmentInfo.target_size().x(),
                        colorAttachmentInfo.size_type() == pb::RenderTarget_Size_WINDOW_SIZE ? windowSize.y : colorAttachmentInfo.target_size().y(),
                        0,
                        attachmentType.format,
                        attachmentType.type,
                        nullptr
                    );

                }
                glFramebufferTexture2D(GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0 + i,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                    colorAttachment, 
                    0);

                glCheckError();
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, attachmentType.internalFormat,
                    colorAttachmentInfo.size_type() == pb::RenderTarget_Size_WINDOW_SIZE ? windowSize.x : colorAttachmentInfo.target_size().x(),
                    colorAttachmentInfo.size_type() == pb::RenderTarget_Size_WINDOW_SIZE ? windowSize.y : colorAttachmentInfo.target_size().y(),
                    0,
                    attachmentType.format,
                    attachmentType.type,
                    nullptr
                );
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, colorAttachment, 0);
                glCheckError();
            }
            const auto& name = colorAttachmentInfo.name();
            if(!name.empty())
            {
                textureMap_[name] = colorAttachment;
            }
        }
    }
    if(framebufferPb.color_attachments_size() == 0)
    {
        constexpr GLenum v = GL_NONE;
        glDrawBuffers(1, &v);
        glReadBuffer(GL_NONE);
        glCheckError();
    }
    if(framebufferPb.has_depth_stencil_attachment())
    {
        const auto& depthStencilAttachmentInfo = framebufferPb.depth_stencil_attachment();
        const auto attachmentType = GetAttachmentType(depthStencilAttachmentInfo);
        if(attachmentType.error != 0)
        {
            LogError("Could not get attachment type for depth stencil attachment");
        }
        const bool stencil = attachmentType.format == GL_DEPTH_STENCIL;
        const auto target = depthStencilAttachmentInfo.cubemap() ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
        if (depthStencilAttachmentInfo.rbo())
        {

            glGenRenderbuffers(1, &depthStencilAttachment_);
            glBindRenderbuffer(GL_RENDERBUFFER, depthStencilAttachment_);
            glRenderbufferStorage(GL_RENDERBUFFER, attachmentType.internalFormat,
                depthStencilAttachmentInfo.size_type() == pb::RenderTarget_Size_WINDOW_SIZE ? windowSize.x : depthStencilAttachmentInfo.target_size().x(),
                depthStencilAttachmentInfo.size_type() == pb::RenderTarget_Size_WINDOW_SIZE ? windowSize.y : depthStencilAttachmentInfo.target_size().y());

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, stencil?GL_DEPTH_STENCIL_ATTACHMENT: GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthStencilAttachment_);
            glCheckError();
        }
        else
        {
            glCreateTextures(target, 1, &depthStencilAttachment_);
            glBindTexture(target, depthStencilAttachment_);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            if(depthStencilAttachmentInfo.cubemap())
            {
                glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                for(int face = 0; face < 6; face++)
                {
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, 0, attachmentType.internalFormat,
                        depthStencilAttachmentInfo.size_type() == pb::RenderTarget_Size_WINDOW_SIZE ? windowSize.x : depthStencilAttachmentInfo.target_size().x(),
                        depthStencilAttachmentInfo.size_type() == pb::RenderTarget_Size_WINDOW_SIZE ? windowSize.y : depthStencilAttachmentInfo.target_size().y(),
                        0,
                        attachmentType.format,
                        attachmentType.type,
                        nullptr
                    );
                    glFramebufferTexture2D(GL_FRAMEBUFFER, stencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, depthStencilAttachment_, 0);

                }
                glCheckError();
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, attachmentType.internalFormat,
                    depthStencilAttachmentInfo.size_type() == pb::RenderTarget_Size_WINDOW_SIZE ? windowSize.x : depthStencilAttachmentInfo.target_size().x(),
                    depthStencilAttachmentInfo.size_type() == pb::RenderTarget_Size_WINDOW_SIZE ? windowSize.y : depthStencilAttachmentInfo.target_size().y(),
                    0,
                    attachmentType.format,
                    attachmentType.type,
                    nullptr
                );

                glFramebufferTexture2D(GL_FRAMEBUFFER, stencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthStencilAttachment_, 0);
                glCheckError();
            }
            const auto& name = depthStencilAttachmentInfo.name();
            if (!name.empty())
            {
                textureMap_[name] = depthStencilAttachment_;
            }
        }
    }

    CheckFramebufferStatus();
    glCheckError();
    Unbind();
}

GLuint Framebuffer::GetTextureName(std::string_view textureName)
{
    const auto it = textureMap_.find(textureName.data());
    if(it != textureMap_.end())
    {
        return it->second;
    }
    return 0;
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

bool CheckFramebufferStatus()
{
    switch(glCheckFramebufferStatus(GL_FRAMEBUFFER))
    {
    case GL_FRAMEBUFFER_COMPLETE:
        LogDebug("Check Framebuffer: Complete");
        return true;
    case GL_FRAMEBUFFER_UNDEFINED:
        LogError("Check Framebuffer: Undefined");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        LogError("Check Framebuffer: Incomplete Attachment");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        LogError("Check Framebuffer: Incomplete Missing Attachment");
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        LogError("Check Framebuffer: Unsupported");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        LogError("Check Framebuffer: Incomplete Multisample");
        break;
    default:
        LogError("Check Framebuffer: Unknown Error");
        break;
    }
    return false;
}
} // namespace gpr5300
