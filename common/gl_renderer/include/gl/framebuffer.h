#pragma once

#include "proto/renderer.pb.h"
#include <GL/glew.h>
#include <glm/vec2.hpp>

#include "renderer/framebuffer.h"

namespace gl
{

struct AttachmentType
{
    GLint internalFormat = 0;
    GLenum format = 0;
    GLenum type = 0;
    GLint error = 0;
};

class Framebuffer : public core::Framebuffer
{
public:
    ~Framebuffer();
    void Bind() const;
    void Resize(glm::uvec2 windowSize);
    void Destroy();
    static void Unbind();
    void Load(const core::pb::FrameBuffer& framebufferPb) override;
    GLuint GetTextureName(std::string_view textureName);
    [[nodiscard]] std::string_view GetName() const { return framebufferName_; }
private:
    GLuint name_ = 0;
    std::string framebufferName_;
    std::vector<GLuint> colorAttachments_{};
    GLuint depthStencilAttachment_ = 0;
    core::pb::FrameBuffer frameBufferPb_;
    std::unordered_map<std::string, GLuint> textureMap_;

    static inline GLuint currentFramebuffer_ = 0;
};

AttachmentType GetAttachmentType(const core::pb::RenderTarget& renderTargetInfo);
bool CheckFramebufferStatus();

} // namespace grp5300
