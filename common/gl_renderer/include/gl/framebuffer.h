#pragma once

#include "proto/renderer.pb.h"
#include <GL/glew.h>
#include <glm/vec2.hpp>

namespace gpr5300::gl
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
    ~Framebuffer();
    void Bind() const;
    void Resize(glm::uvec2 windowSize);
    void Destroy();
    static void Unbind();
    void Load(const pb::FrameBuffer& framebufferPb);
    GLuint GetTextureName(std::string_view textureName);
    [[nodiscard]] std::string_view GetName() const { return framebufferName_; }
private:
    GLuint name_ = 0;
    std::string framebufferName_;
    std::vector<GLuint> colorAttachments_{};
    GLuint depthStencilAttachment_ = 0;
    pb::FrameBuffer frameBufferPb_;
    std::unordered_map<std::string, GLuint> textureMap_;

    static inline GLuint currentFramebuffer_ = 0;
};

AttachmentType GetAttachmentType(const pb::RenderTarget& renderTargetInfo);
bool CheckFramebufferStatus();

} // namespace grp5300
