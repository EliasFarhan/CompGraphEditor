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

class Framebuffer
{
public:
    void Bind() const;
    static void Unbind();
    void Load(const pb::FrameBuffer& framebufferPb);
    GLuint GetTextureName(std::string_view textureName);
private:
    GLuint name = 0;
    std::vector<GLuint> colorAttachments_{};
    GLuint depthStencilAttachment = 0;
    std::unordered_map<std::string, GLuint> textureMap_;
    static inline GLuint currentFramebuffer_ = 0;
};

AttachmentType GetAttachmentType(const pb::RenderTarget& renderTargetInfo);
bool CheckFramebufferStatus();

} // namespace grp5300
