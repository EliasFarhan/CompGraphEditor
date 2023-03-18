#include "gl/material.h"

#include "engine/scene.h"
#include "gl/pipeline.h"

namespace gl
{
void Material::Bind()
{
    auto& textureManager = static_cast<TextureManager&>(core::GetTextureManager());
    auto& pipeline = core::GetCurrentScene()->GetPipeline(pipelineIndex);
    pipeline.Bind();
    auto& glPipeline = static_cast<Pipeline&>(pipeline);
    for (std::size_t textureIndex = 0; textureIndex < textures.size(); textureIndex++)
    {
        if (textures[textureIndex].textureId == core::INVALID_TEXTURE_ID)
            continue;
        glPipeline.SetTexture(
            textures[textureIndex].uniformSamplerName,
            textureManager.GetTexture(textures[textureIndex].textureId),
            textureIndex);
    }
}
} // namespace gl
