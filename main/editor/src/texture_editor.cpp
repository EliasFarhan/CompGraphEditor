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
    const auto shaderPath = "data/shaders/pre_compute_brdf.comp";
    auto computeShader = filesystem.LoadFile(shaderPath);
    GLuint brdfShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(brdfShader, 1, reinterpret_cast<char**>(&computeShader.data), nullptr);
    glCompileShader(brdfShader);
    // check for compilation errors as per normal here
    //Check success status of shader compilation
    GLint success;
    glGetShaderiv(brdfShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        constexpr GLsizei infoLogSize = 512;
        char infoLog[infoLogSize];
        glGetShaderInfoLog(brdfShader, infoLogSize, nullptr, infoLog);
        LogError(fmt::format("Shader compilation failed with this log:\n{}\nShader Path:\n{}",
            infoLog, shaderPath));
        glDeleteShader(brdfShader);
        return;
    }
    GLuint brdfProgram = glCreateProgram();
    glAttachShader(brdfProgram, brdfShader);
    glLinkProgram(brdfProgram);
    // check for linking errors and validate program as per normal here
    //Check if shader program was linked correctly
    glGetProgramiv(brdfProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        constexpr GLsizei infoLogSize = 512;
        char infoLog[infoLogSize];
        glGetProgramInfoLog(brdfProgram, infoLogSize, nullptr, infoLog);
        LogError(fmt::format("Shader program: LINK_FAILED with infoLog:\n{}",
            infoLog));
        return;
    }

    glDeleteShader(brdfShader);
    { // launch compute shaders!
        glUseProgram(brdfProgram);
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
    glUseProgram(0);
    glDeleteProgram(brdfProgram);
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
        glBindImageTexture(0, envMap, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        stbi_image_free(envMapData);
    }
    else
    {
        //Error loading hdr
        return;
    }
    

    //TODO from equirectangle to cubemap

    //TODO generate irradiance cubemap

    //TODO from cubemap to equirectangle

    //TODO export as hdr

    //TODO clean textures
    glDeleteTextures(1, &envMap);
}

void TextureEditor::GeneratePreFilterEnvMap(std::string_view path)
{
    //TODO from equirectangle to cubemap

    //TODO generate prefilter cubemap

    //TODO from cubemap to equirectangle

    //TODO export as hdr
}
}
