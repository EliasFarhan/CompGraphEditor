#include "texture_editor.h"
#include "utils/log.h"
#include "editor.h"
#include "material_editor.h"
#include <imgui.h>
#include <fmt/format.h>
#include <array>
#include <fstream>

#include "engine/filesystem.h"
#include "gl/debug.h"

#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "pbr_utils.h"
#include "gl/framebuffer.h"
#include "gl/pipeline.h"
#include "gl/shape_primitive.h"
#include "gl/texture.h"


namespace editor
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
        static constexpr std::array<std::string_view, core::pb::Cubemap::LENGTH> cubemapSides
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

        if(ImGui::Button("Convert To KTX"))
        {
            CubeToKtx(currentTextureInfo);
        }

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
                currentTextureInfo.info.set_wrapping_mode(static_cast<core::pb::Texture_WrappingMode>(i));
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
                currentTextureInfo.info.set_filter_mode(static_cast<core::pb::Texture_FilteringMode>(i));
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
        if(ImGui::Button("HDR to KTX"))
        {
            HdrToKtx(currentTextureInfo);
        }
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
    auto& filesystem = core::FilesystemLocator::get();
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
    static constexpr std::array<std::string_view, 9> extensions
    {
        ".jpg",
        ".jpeg",
        ".png",
        ".bmp",
        ".tga",
        ".hdr",
        ".gif",
        ".cube",
        ".ktx"
    };
    return extensions;
}

void TextureEditor::CubeToKtx(const TextureInfo& textureInfo)
{

    const auto ktxPath = fmt::format("{}/{}.ktx", GetFolder(textureInfo.info.path()), GetFilename(textureInfo.info.path(), false));

    gl::Texture cubemap;
    cubemap.LoadCubemap(textureInfo.info);


    const auto& firstFacePath = textureInfo.cubemap.texture_paths(0);
    int w, h, channelCount;
    if(!stbi_info(firstFacePath.data(), &w, &h, &channelCount))
    {
        LogError(fmt::format("KTX conversion: Could not get info for face 0: {}", firstFacePath));
        return;
    }
    ktxTexture1* texture;                   
    ktxTextureCreateInfo createInfo;
    KTX_error_code result;

    createInfo.glInternalformat = GL_RGB8;
    createInfo.baseWidth = w;
    createInfo.baseHeight = h;
    createInfo.baseDepth = 1;
    createInfo.numDimensions = 2;
    createInfo.numLevels = 1 ;
    createInfo.numLayers = 1;
    createInfo.numFaces = 6;
    createInfo.isArray = KTX_FALSE;
    createInfo.generateMipmaps = textureInfo.info.generate_mipmaps();
    

    // Call ktxTexture1_Create to create a KTX texture.
    result = ktxTexture1_Create(&createInfo,
        KTX_TEXTURE_CREATE_ALLOC_STORAGE,
        &texture);
    if(!gl::CheckKtxError(result))
    {
        return;
    }
    const int size = w * h * channelCount;
    void* buffer = std::malloc(size);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.name);
    for (int faceIndex = 0; faceIndex < 6; faceIndex++)
    {
        glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X+faceIndex, 0, textureInfo.info.gamma_correction()?GL_SRGB:GL_RGB, GL_UNSIGNED_BYTE, buffer);
        result = ktxTexture_SetImageFromMemory(ktxTexture(texture),
            0, 0, faceIndex,
            static_cast<const ktx_uint8_t*>(buffer), size);
        gl::CheckKtxError(result);
    }
    std::free(buffer);
    

    // Repeat for the other 15 slices of the base level and all other levels
    // up to createInfo.numLevels.
    
    ktxTexture_WriteToNamedFile(ktxTexture(texture), ktxPath.data());
    ktxTexture_Destroy(ktxTexture(texture));
    cubemap.Destroy();

    auto& resourceManager = Editor::GetInstance()->GetResourceManager();
    resourceManager.AddResource(ktxPath);

    const auto ktxId = resourceManager.FindResourceByPath(ktxPath);
    auto* ktxTextureInfo = GetTexture(ktxId);
    ktxTextureInfo->info.set_filter_mode(textureInfo.info.filter_mode());
    ktxTextureInfo->info.set_wrapping_mode(textureInfo.info.wrapping_mode());
    ktxTextureInfo->info.set_gamma_correction(textureInfo.info.gamma_correction());
}

void TextureEditor::HdrToKtx(const TextureInfo& textureInfo)
{
    const auto& path = textureInfo.info.path();
    const auto baseDir = GetFolder(path);
    const auto filename = GetFilename(path, false);
    const auto ktxMapPath = fmt::format("{}/{}.ktx", baseDir, filename);

    auto& filesystem = core::FilesystemLocator::get();
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

    const auto targetSize = texH / 2;
    auto cube = gl::GenerateCube(glm::vec3(2.0f), glm::vec3(0.0f));

    core::pb::FrameBuffer captureFboInfo;
    captureFboInfo.set_name("captureFBO");
    auto* captureCubemap = captureFboInfo.add_color_attachments();
    captureCubemap->set_cubemap(true);
    captureCubemap->set_type(core::pb::RenderTarget_Type_FLOAT);
    captureCubemap->set_format(core::pb::RenderTarget_Format_RGBA);
    captureCubemap->set_format_size(core::pb::RenderTarget_FormatSize_SIZE_32);
    captureCubemap->set_size_type(core::pb::RenderTarget_Size_FIXED_SIZE);
    captureCubemap->mutable_target_size()->set_x(targetSize);
    captureCubemap->mutable_target_size()->set_y(targetSize);
    static constexpr std::string_view envCubemapName = "envCubeName";
    captureCubemap->set_name(envCubemapName.data());

    gl::Framebuffer captureFbo;
    captureFbo.Load(captureFboInfo);

    //Generate environment cubemap
    //from equirectangle to cubemap
    core::pb::Shader cubemapShaderInfo;
    cubemapShaderInfo.set_path("shaders/cubemap.vert");
    cubemapShaderInfo.set_type(core::pb::Shader_Type_VERTEX);

    gl::Shader cubemapShader;
    cubemapShader.LoadShader(cubemapShaderInfo);

    core::pb::Shader equirectangleToCubemapShaderInfo;
    equirectangleToCubemapShaderInfo.set_path("shaders/equirectangle_to_cubemap.frag");
    equirectangleToCubemapShaderInfo.set_type(core::pb::Shader_Type_FRAGMENT);

    gl::Shader equirectangleToCubemapShader;
    equirectangleToCubemapShader.LoadShader(equirectangleToCubemapShaderInfo);

    gl::Pipeline equirectangleToCubemap;
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

    glViewport(0, 0, targetSize, targetSize);
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
    equirectangleToCubemap.Unbind();
    captureFbo.Unbind();
    glCheckError();

    //Write to KTX
    ktxTexture1* texture;
    ktxTextureCreateInfo createInfo;
    KTX_error_code result;

    createInfo.glInternalformat = GL_RGBA32F;
    createInfo.baseWidth = targetSize;
    createInfo.baseHeight = targetSize;
    createInfo.baseDepth = 1;
    createInfo.numDimensions = 2;
    createInfo.numLevels = 1;
    createInfo.numLayers = 1;
    createInfo.numFaces = 6;
    createInfo.isArray = KTX_FALSE;
    createInfo.generateMipmaps = textureInfo.info.generate_mipmaps();

    
    result = ktxTexture1_Create(&createInfo,
        KTX_TEXTURE_CREATE_ALLOC_STORAGE,
        &texture);
    if (!gl::CheckKtxError(result))
    {
        return;
    }
    const int size = targetSize * targetSize * 4 * 4;
    void* buffer = std::malloc(size);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (int faceIndex = 0; faceIndex < 6; faceIndex++)
    {
        glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex, 0, GL_RGBA, GL_FLOAT, buffer);
        result = ktxTexture_SetImageFromMemory(ktxTexture(texture),
            0, 0, faceIndex,
            static_cast<const ktx_uint8_t*>(buffer), size);
        gl::CheckKtxError(result);
        glCheckError();
    }
    
    std::free(buffer);


    // Repeat for the other 15 slices of the base level and all other levels
    // up to createInfo.numLevels.

    ktxTexture_WriteToNamedFile(ktxTexture(texture), ktxMapPath.data());
    ktxTexture_Destroy(ktxTexture(texture));

    auto& resourceManager = Editor::GetInstance()->GetResourceManager();
    resourceManager.AddResource(ktxMapPath);

    const auto ktxId = resourceManager.FindResourceByPath(ktxMapPath);
    auto* ktxTextureInfo = GetTexture(ktxId);
    ktxTextureInfo->info.set_filter_mode(textureInfo.info.filter_mode());
    ktxTextureInfo->info.set_wrapping_mode(textureInfo.info.wrapping_mode());
    ktxTextureInfo->info.set_gamma_correction(textureInfo.info.gamma_correction());
    glCheckError();

    captureFbo.Destroy();
    equirectangleToCubemap.Destroy();
}
}
