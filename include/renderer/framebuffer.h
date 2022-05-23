#pragma once

#include "proto/renderer.pb.h"
#include <GL/glew.h>

namespace gpr5300
{

struct AttachmentType
{
    GLint internalFormat = 0;
    GLenum format = 0;
    GLenum type = 0;
    GLint error = 0;
};

struct Framebuffer
{
    void Load(const pb::FrameBuffer& framebufferPb);
    GLuint name = 0;
    std::vector<GLuint> colorAttachements;
    GLuint depthStencilAttachment;
};

AttachmentType GetAttachmentType(const pb::RenderTarget& renderTargetInfo);


} // namespace grp5300
