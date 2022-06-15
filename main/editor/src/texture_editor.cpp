#include "texture_editor.h"
#include "utils/log.h"
#include "editor.h"
#include "material_editor.h"
#include <imgui.h>
#include <fmt/format.h>
#include <array>
#include <fstream>

#include "engine/filesystem.h"
#include "renderer/debug.h"

#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "renderer/shape_primitive.h"

namespace gpr5300
{

void TextureEditor::DrawInspector()
{
    if (currentIndex_ >= textureInfos_.size())
    {
        return;
    }
    auto& currentTextureInfo = textureInfos_[currentIndex_];
    ImGui::Text("Filename: %s", currentTextureInfo.filename.c_str());

    if(GetFileExtension(currentTextureInfo.filename) == ".cube")
    {
        //choose cubemap textures
        static constexpr std::array<std::string_view, pb::Cubemap::LENGTH> cubemapSides
        {
            "Right Side",
            "Left Side",
            "Top Side",
            "Bottom Side",
            "Front Side",
            "Back Side",
        };
        ImGui::PushID("Cubemap textures");
        for(std::size_t i = 0; i < cubemapSides.size(); i++)
        {
            ImGui::PushID(i);
            if(currentTextureInfo.cubemap.texture_paths_size() <= i)
            {
                currentTextureInfo.cubemap.add_texture_paths();
            }
            auto* texturePath = currentTextureInfo.cubemap.mutable_texture_paths(i);
            if(ImGui::BeginCombo(cubemapSides[i].data(), texturePath->empty()? "No texture": texturePath->data()))
            {
                for(std::size_t textureIndex = 0; textureIndex < textureInfos_.size(); textureIndex++)
                {
                    auto& otherTexturePath = textureInfos_[textureIndex].info.path();
                    if(GetFileExtension(otherTexturePath) == ".cube")
                    {
                        continue;
                    }
                    const bool selected = !texturePath->empty() && fs::equivalent(*texturePath, otherTexturePath);
                    if(ImGui::Selectable(textureInfos_[textureIndex].filename.data(), selected))
                    {
                        *texturePath = otherTexturePath;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopID();
        }
        ImGui::PopID();
    }

    static constexpr std::array<std::string_view, 4> wrappingModeNames
    {
        "REPEAT",
        "MIRROR_REPEAT",
        "CLAMP_TO_EDGE",
        "CLAMP_TO_BORDER"
    };
    if(ImGui::BeginCombo("Wrapping Mode", wrappingModeNames[currentTextureInfo.info.wrapping_mode()].data()))
    {
        for(std::size_t i = 0; i < wrappingModeNames.size(); ++i)
        {
            if(ImGui::Selectable(wrappingModeNames[i].data(), i == currentTextureInfo.info.wrapping_mode()))
            {
                currentTextureInfo.info.set_wrapping_mode(static_cast<pb::Texture_WrappingMode>(i));
            }
        }
        ImGui::EndCombo();
    }
    static constexpr std::array<std::string_view, 2> filterModeNames
    {
        "NEAREST",
        "LINEAR"
    };
    if (ImGui::BeginCombo("Filter Mode", filterModeNames[currentTextureInfo.info.filter_mode()].data()))
    {
        for (std::size_t i = 0; i < filterModeNames.size(); ++i)
        {
            if (ImGui::Selectable(filterModeNames[i].data(), i == currentTextureInfo.info.filter_mode()))
            {
                currentTextureInfo.info.set_filter_mode(static_cast<pb::Texture_FilteringMode>(i));
            }
        }
        ImGui::EndCombo();
    }

    bool generateMipMaps = currentTextureInfo.info.generate_mipmaps();
    if(ImGui::Checkbox("Generate Mip Map", &generateMipMaps))
    {
        currentTextureInfo.info.set_generate_mipmaps(generateMipMaps);
    }
    bool gammaCorrection = currentTextureInfo.info.gamma_correction();
    if(ImGui::Checkbox("Gamma Correction", &gammaCorrection))
    {
        currentTextureInfo.info.set_gamma_correction(gammaCorrection);
    }

    if(GetFileExtension(currentTextureInfo.info.path()) == ".hdr")
    {
        if(ImGui::Button("Generate Irradiance Map"))
        {
            GenerateIrradianceMap(currentTextureInfo.info.path());
        }
        if(ImGui::Button("Generate Pre-Filter Environment Map"))
        {
            GeneratePreFilterEnvMap(currentTextureInfo.info.path());
        }
    }
}

bool TextureEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = textureInfos_.size();
    for (std::size_t i = 0; i < textureInfos_.size(); i++)
    {
        const auto& shaderInfo = textureInfos_[i];
        if (ImGui::Selectable(shaderInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}

std::string_view TextureEditor::GetSubFolder()
{
    return "textures/";
}

EditorType TextureEditor::GetEditorType()
{
    return EditorType::TEXTURE;
}

void TextureEditor::Save()
{
    for(auto& textureInfo : textureInfos_)
    {
        if(GetFileExtension(textureInfo.info.path()) == ".cube")
        {
            std::ofstream cubeOut(textureInfo.info.path(), std::ios::binary);
            if (!textureInfo.cubemap.SerializeToOstream(&cubeOut))
            {
                LogWarning(fmt::format("Could not save cubemap info at: {}", textureInfo.info.path()));
            }
        }
        std::ofstream fileOut(textureInfo.infoPath, std::ios::binary);
        if (!textureInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save texture info at: {}", textureInfo.infoPath));
        }
    }
}

void TextureEditor::AddResource(const Resource &resource)
{
    auto& filesystem = FilesystemLocator::get();
    TextureInfo textureInfo{};
    textureInfo.resourceId = resource.resourceId;
    textureInfo.filename = GetFilename(resource.path);
    textureInfo.info.set_path(resource.path);
    textureInfo.infoPath = resource.path + ".meta";
    if(filesystem.FileExists(textureInfo.infoPath))
    {
        std::ifstream metaFile(textureInfo.infoPath,std::ios::binary);
        textureInfo.info.ParseFromIstream(&metaFile);
    }
    if(GetFileExtension(resource.path) == ".cube")
    {
        std::ifstream cubemapFile(resource.path, std::ios::binary);
        textureInfo.cubemap.ParseFromIstream(&cubemapFile);
    }
    textureInfos_.push_back(textureInfo);
}

void TextureEditor::RemoveResource(const Resource &resource)
{
    const auto it = std::ranges::find_if(textureInfos_, [&resource](const auto& texture)
    {
        return resource.resourceId == texture.resourceId;
    });
    if(it != textureInfos_.end())
    {
        textureInfos_.erase(it);
        const auto* editor = Editor::GetInstance();
        auto* materialEditor = dynamic_cast<MaterialEditor*>(editor->GetEditorSystem(EditorType::MATERIAL));

        materialEditor->RemoveResource(resource);
    }
}

void TextureEditor::UpdateExistingResource(const Resource &resource)
{

}

void TextureEditor::ReloadId()
{
}

TextureInfo* TextureEditor::GetTexture(ResourceId resourceId)
{
    const auto it = std::ranges::find_if(textureInfos_, [resourceId](const auto& textureInfo)
        {
            return resourceId == textureInfo.resourceId;
        });
    if(it != textureInfos_.end())
    {
        return &*it;
    }
    return nullptr;
}

void TextureEditor::Delete()
{
    if (currentIndex_ >= textureInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.RemoveResource(textureInfos_[currentIndex_].info.path());
}

std::span<const std::string_view> TextureEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 8> extensions
    {
        ".jpg",
        ".jpeg",
        ".png",
        ".bmp",
        ".tga",
        ".hdr",
        ".gif",
        ".cube"
    };
    return extensions;
}

void TextureEditor::GeneratePreComputeBrdfLUT()
{
    constexpr int texW = 512;
    // dimensions of the image
    constexpr int texH = 512;
    GLuint texOutput;
    glGenTextures(1, &texOutput);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texW, texH, 0, GL_RGBA, GL_FLOAT,
        nullptr);
    glBindImageTexture(0, texOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glCheckError();

    const auto& filesystem = FilesystemLocator::get();

    pb::Shader shaderInfo;
    shaderInfo.set_path("shaders/pre_compute_brdf.comp");
    shaderInfo.set_type(pb::Shader_Type_COMPUTE);

    Shader shader;
    shader.LoadShader(shaderInfo);
    Pipeline pipeline;
    pipeline.LoadComputePipeline(shader);

    shader.Destroy();
    { // launch compute shaders!
        pipeline.Bind();
        glDispatchCompute(static_cast<GLuint>(texW), static_cast<GLuint>(texH), 1);
    }

    // make sure writing to image has finished before read
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glCheckError();
    auto* buffer = static_cast<float*>(std::calloc(texH * texW, 4*sizeof(float)));
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);
    glCheckError();
    stbi_flip_vertically_on_write(true);
    if(!stbi_write_hdr("data/textures/brdf_lut.hdr", texW, texH, 4, buffer))
    {
        //Error
        LogError("Error while exporting BRDF LUT to hdr texture");
    }
    std::free(buffer);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &texOutput);
    pipeline.Unbind();
    pipeline.Destroy();
    glCheckError();
}

void TextureEditor::GenerateIrradianceMap(std::string_view path)
{
    const auto baseDir = gpr5300::GetFolder(path);
    const auto filename = gpr5300::GetFilename(path, false);
    const auto irradianceMapPath = fmt::format("{}/{}_irrmap.hdr", baseDir, filename);

    auto& filesystem = FilesystemLocator::get();
    auto envMapFile = filesystem.LoadFile(path);
    int texW;
    int texH;
    int channel;

    stbi_set_flip_vertically_on_load(true);
    auto* envMapData = stbi_loadf_from_memory(envMapFile.data, envMapFile.length, &texW, &texH, &channel, 4);
    
    unsigned int envMap;
    if (envMapData)
    {
        glGenTextures(1, &envMap);
        glBindTexture(GL_TEXTURE_2D, envMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texW, texH, 0, GL_RGBA, GL_FLOAT, envMapData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glBindImageTexture(0, envMap, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(envMapData);
        
    }
    else
    {
        //Error loading hdr
        return;
    }
    glCheckError();
    auto cube = GenerateCube(glm::vec3(2.0f), glm::vec3(0.0f));

    pb::FrameBuffer captureFboInfo;
    captureFboInfo.set_name("captureFBO");
    auto* captureCubemap = captureFboInfo.add_color_attachments();
    captureCubemap->set_cubemap(true);
    captureCubemap->set_type(pb::RenderTarget_Type_FLOAT);
    captureCubemap->set_format(pb::RenderTarget_Format_RGBA);
    captureCubemap->set_format_size(pb::RenderTarget_FormatSize_SIZE_32);
    captureCubemap->set_size_type(pb::RenderTarget_Size_FIXED_SIZE);
    captureCubemap->mutable_target_size()->set_x(512);
    captureCubemap->mutable_target_size()->set_y(512);
    static constexpr std::string_view envCubemapName = "envCubeName";
    captureCubemap->set_name(envCubemapName.data());

    Framebuffer captureFbo;
    captureFbo.Load(captureFboInfo);

    //Generate environment cubemap
    //from equirectangle to cubemap
    pb::Shader cubemapShaderInfo;
    cubemapShaderInfo.set_path("shaders/cubemap.vert");
    cubemapShaderInfo.set_type(pb::Shader_Type_VERTEX);

    Shader cubemapShader;
    cubemapShader.LoadShader(cubemapShaderInfo);

    pb::Shader equirectangleToCubemapShaderInfo;
    equirectangleToCubemapShaderInfo.set_path("shaders/equirectangle_to_cubemap.frag");
    equirectangleToCubemapShaderInfo.set_type(pb::Shader_Type_FRAGMENT);

    Shader equirectangleToCubemapShader;
    equirectangleToCubemapShader.LoadShader(equirectangleToCubemapShaderInfo);

    Pipeline equirectangleToCubemap;
    equirectangleToCubemap.LoadRasterizePipeline(cubemapShader, equirectangleToCubemapShader);
    equirectangleToCubemapShader.Destroy();

    equirectangleToCubemap.Bind();
    equirectangleToCubemap.SetTexture("equirectangularMap", envMap, 0);
    glBindVertexArray(cube.vao);
    glCheckError();
    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
    // ----------------------------------------------------------------------------------------------
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    equirectangleToCubemap.SetMat4("projection", captureProjection);

    glViewport(0, 0, 512, 512);
    captureFbo.Bind();
    glCheckError();
    auto envCubemap = captureFbo.GetTextureName(envCubemapName);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glCheckError();
        equirectangleToCubemap.SetMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glCheckError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCheckError();
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
        glCheckError();
    }
    captureFbo.Unbind();

    //generate irradiance cubemap
    pb::FrameBuffer irradianceFboInfo;
    irradianceFboInfo.set_name("irradianceFbo");
    auto* irradianceAttachmentInfo = irradianceFboInfo.add_color_attachments();
    irradianceAttachmentInfo->set_type(pb::RenderTarget_Type_FLOAT);
    irradianceAttachmentInfo->set_format(pb::RenderTarget_Format_RGBA);
    irradianceAttachmentInfo->set_format_size(pb::RenderTarget_FormatSize_SIZE_32);
    irradianceAttachmentInfo->set_size_type(pb::RenderTarget_Size_FIXED_SIZE);
    irradianceAttachmentInfo->set_cubemap(true);
    static constexpr std::string_view irradianceMapName = "irradiance";
    irradianceAttachmentInfo->set_name(irradianceMapName.data());
    irradianceAttachmentInfo->mutable_target_size()->set_x(32);
    irradianceAttachmentInfo->mutable_target_size()->set_y(32);

    Framebuffer irradianceFbo;
    irradianceFbo.Load(irradianceFboInfo);

    pb::Shader irradianceConvolutionShaderInfo;
    irradianceConvolutionShaderInfo.set_type(pb::Shader_Type_FRAGMENT);
    irradianceConvolutionShaderInfo.set_path("shaders/irradiance_convolution.frag");

    Shader irradianceConvlutionShader;
    irradianceConvlutionShader.LoadShader(irradianceConvolutionShaderInfo);

    Pipeline irradianceConvolution;
    irradianceConvolution.LoadRasterizePipeline(cubemapShader, irradianceConvlutionShader);

    irradianceFbo.Bind();
    irradianceConvolution.Bind();
    irradianceConvolution.SetMat4("projection", captureProjection);
    glBindVertexArray(cube.vao);
    irradianceConvolution.SetInt("environmentMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, captureFbo.GetTextureName(envCubemapName));
    glViewport(0, 0, 32, 32);
    glCheckError();
    const auto irradianceMap = irradianceFbo.GetTextureName(irradianceMapName);
    for(int face = 0; face < 6; face++)
    {
        irradianceConvolution.SetMat4("view", captureViews[face]);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, irradianceMap, 0);
        glCheckError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCheckError();
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
        glCheckError();
    }
    irradianceFbo.Unbind();
    irradianceConvolution.Unbind();


    //from cubemap to equirectangle

    pb::Shader equirectangleVertShaderInfo;
    equirectangleVertShaderInfo.set_path("shaders/equirectangle.vert");
    equirectangleVertShaderInfo.set_type(pb::Shader_Type_VERTEX);

    pb::Shader equirectangleFragShaderInfo;
    equirectangleFragShaderInfo.set_path("shaders/equirectangle.frag");
    equirectangleFragShaderInfo.set_type(pb::Shader_Type_FRAGMENT);

    Shader equirectangleVertShader;
    equirectangleVertShader.LoadShader(equirectangleVertShaderInfo);

    Shader equirectangleFragShader;
    equirectangleFragShader.LoadShader(equirectangleFragShaderInfo);

    Pipeline equirectangle;
    equirectangle.LoadRasterizePipeline(equirectangleVertShader, equirectangleFragShader);

    auto resultW = 32*4;
    auto resultH = 32*2;

    pb::FrameBuffer equirectangleFboInfo;
    auto* equirectangleColorAttachment = equirectangleFboInfo.add_color_attachments();
    equirectangleColorAttachment->set_name("irradiance");
    equirectangleColorAttachment->set_type(pb::RenderTarget_Type_FLOAT);
    equirectangleColorAttachment->set_format(pb::RenderTarget_Format_RGBA);
    equirectangleColorAttachment->set_size_type(pb::RenderTarget_Size_FIXED_SIZE);
    equirectangleColorAttachment->set_format_size(pb::RenderTarget_FormatSize_SIZE_32);
    equirectangleColorAttachment->mutable_target_size()->set_x(resultW);
    equirectangleColorAttachment->mutable_target_size()->set_y(resultH);

    auto quad = GenerateQuad(glm::vec3(2.0f), glm::vec3(0.0f));

    Framebuffer equirectangleFbo;
    equirectangleFbo.Load(equirectangleFboInfo);

    equirectangleFbo.Bind();
    equirectangle.Bind();
    glBindVertexArray(quad.vao);
    glViewport(0, 0, resultW, resultH);

    equirectangle.SetInt("irradiance", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceFbo.GetTextureName(irradianceMapName));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    equirectangleFbo.Unbind();
    equirectangle.Unbind();

    //export as hdr
    glBindTexture(GL_TEXTURE_2D, equirectangleFbo.GetTextureName("irradiance"));
    auto* buffer = static_cast<float*>(std::calloc(resultW * resultH, 4 * sizeof(float)));
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);
    glCheckError();
    stbi_flip_vertically_on_write(true);
    //TODO give proper name
    if (!stbi_write_hdr(irradianceMapPath.data(), resultW, resultH, 4, buffer))
    {
        //Error
        LogError("Error while exporting Irradiance map to hdr texture");
    }
    std::free(buffer);

    //TODO clean textures
    equirectangle.Destroy();
    equirectangleFbo.Destroy();
    irradianceConvolution.Destroy();
    irradianceConvlutionShader.Destroy();
    cubemapShader.Destroy();
    equirectangleToCubemap.Destroy();
    captureFbo.Destroy();
    irradianceFbo.Destroy();
    glDeleteTextures(1, &envMap);
    glDeleteVertexArrays(1, &cube.vao);
    glDeleteVertexArrays(1, &quad.vao);
    glCheckError();
}

void TextureEditor::GeneratePreFilterEnvMap(std::string_view path)
{
    //TODO from equirectangle to cubemap

    //TODO generate prefilter cubemap

    //TODO from cubemap to equirectangle

    //TODO export as hdr
}
}
