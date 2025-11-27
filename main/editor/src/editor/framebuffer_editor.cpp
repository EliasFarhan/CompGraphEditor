#include "framebuffer_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"
#include "editor.h"

#include <format>
#include <imgui_stdlib.h>
#include <array>
#include <fstream>

#include "utils/fb_file.h"


namespace novus::editor
{
void FramebufferEditor::AddResource(const Resource& resource)
{
    FramebufferInfo framebufferInfo{};
    framebufferInfo.filename = GetFilename(resource.path);
    framebufferInfo.resourceId = resource.resourceId;
    framebufferInfo.path = resource.path;


    if (!core::IsRegularFile(resource.path))
    {
        LogWarning(std::format("Could not find framebuffer file: {}", resource.path.c_str()));
        return;
    }

    if (!core::ReadFlatbufferFromFile<renderer::FramebufferT, renderer::Framebuffer>(resource.path, framebufferInfo.info))
    {
        LogWarning(std::format("Could not open protobuf file: {}", resource.path.c_str()));
        return;
    }
    if(framebufferInfo.info.name.empty())
    {
        framebufferInfo.info.name = (GetFilename(resource.path, false));
    }
    framebufferInfos_.push_back(framebufferInfo);
}

void FramebufferEditor::RemoveResource(const Resource& resource)
{
    const auto it = std::ranges::find_if(framebufferInfos_, [&resource](const auto& framebufferInfo)
        {
            return resource.resourceId == framebufferInfo.resourceId;
        });
    if (it != framebufferInfos_.end())
    {
        framebufferInfos_.erase(it);
        const auto* editor = Editor::GetInstance();
        auto* renderPassEditor = editor->GetEditorSystem(EditorType::RENDER_PASS);
        renderPassEditor->RemoveResource(resource);
    }
}

void FramebufferEditor::UpdateExistingResource(const Resource& resource)
{
}


void FramebufferEditor::DrawInspector()
{
    if(currentIndex_ >= framebufferInfos_.size())
    {
        return;
    }

    auto* editor = Editor::GetInstance();
    auto& currentFramebufferInfo = framebufferInfos_[currentIndex_];
    ImGui::InputText("Framebuffer Name", &currentFramebufferInfo.info.name);
    ImGui::Separator();
    int deletedColorAttachment = -1;
    for(size_t colorAttachmentIndex = 0; colorAttachmentIndex < currentFramebufferInfo.info.color_attachments.size(); colorAttachmentIndex++)
    {
        auto& colorAttachment = currentFramebufferInfo.info.color_attachments[colorAttachmentIndex];
        std::string id = std::format("Color Attachment {}", colorAttachmentIndex);
        std::string colorAttachmentName = colorAttachment.name.empty() ? id : colorAttachment.name;
        ImGui::PushID(id.data());
        if(ImGui::InputText("Color Attachment Name", &colorAttachmentName))
        {
            colorAttachment.name = colorAttachmentName;
        }

        if(ImGui::BeginCombo("Format",internal::EnumNameTextureFormat(colorAttachment.texture_info->format)))
        {
            for(std::size_t format = 0; format <= internal::TextureFormat_MAX; format++)
            {
                if(ImGui::Selectable(internal::EnumNameTextureFormat((internal::TextureFormat)format),
                    format == colorAttachment.texture_info->format))
                {
                    colorAttachment.texture_info->format = (internal::TextureFormat)format;
                }
            }
            ImGui::EndCombo();
        }

        bool fixedSize = colorAttachment.texture_info->width != 0;
        if(ImGui::Checkbox("Fixed Size", &fixedSize))
        {
            if (fixedSize)
            {
                colorAttachment.texture_info->width = 512;
                colorAttachment.texture_info->height = 512;
            }
            else
            {
                colorAttachment.texture_info->width = 0;
                colorAttachment.texture_info->height = 0;
            }
        }
        if(fixedSize)
        {
            std::array targetSize{(int)colorAttachment.texture_info->width, (int)colorAttachment.texture_info->height};
            if(ImGui::InputInt2("Target Size", targetSize.data()))
            {
                colorAttachment.texture_info->width = (targetSize[0]);
                colorAttachment.texture_info->height = (targetSize[1]);
            }
        }




        if(ImGui::Button("Remove Color Attachment"))
        {
            deletedColorAttachment = colorAttachmentIndex;
        }

        //TODO add sampler info
        
        ImGui::PopID();
        ImGui::Separator();
    }
    if(deletedColorAttachment != -1)
    {
        currentFramebufferInfo.info.color_attachments.erase(currentFramebufferInfo.info.color_attachments.begin()+deletedColorAttachment);
    }
    if(ImGui::Button("Add Color Attachment"))
    {
        renderer::ColorAttachmentT newColorAttachment{};
        newColorAttachment.texture_info = std::make_unique<internal::TextureInfoT>();
        newColorAttachment.target_info = std::make_unique<internal::ColorTargetInfoT>();
        currentFramebufferInfo.info.color_attachments.push_back(std::move(newColorAttachment));
    }
    //depth/stencil attachment
    if(currentFramebufferInfo.info.depth_stencil_attachment != nullptr)
    {
        auto* depthStencilAttachment = currentFramebufferInfo.info.depth_stencil_attachment.get();
        std::string id = "Depth Stencil Attachment";
        std::string depthStencilAttachmentName = depthStencilAttachment->name.empty() ? id : depthStencilAttachment->name;
        if (ImGui::InputText("Depth Stencil Attachment Name", &depthStencilAttachmentName))
        {
            depthStencilAttachment->name = depthStencilAttachmentName;
        }
        bool stencil = depthStencilAttachment->texture_info->format == internal::TextureFormat_TEXTUREFORMAT_D24_UNORM_S8_UINT ||
            internal::TextureFormat_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
        if(ImGui::Checkbox("Stencil", &stencil))
        {
            if (stencil)
            {
                switch (depthStencilAttachment->texture_info->format)
                {
                case internal::TextureFormat_TEXTUREFORMAT_D16_UNORM:
                case internal::TextureFormat_TEXTUREFORMAT_D24_UNORM:
                {
                    depthStencilAttachment->texture_info->format = internal::TextureFormat_TEXTUREFORMAT_D24_UNORM_S8_UINT;
                    break;
                }
                case internal::TextureFormat_TEXTUREFORMAT_D32_FLOAT:
                {
                    depthStencilAttachment->texture_info->format = internal::TextureFormat_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
                    break;
                }
                default:
                {
                    depthStencilAttachment->texture_info->format = internal::TextureFormat_TEXTUREFORMAT_D24_UNORM_S8_UINT;
                    break;
                }
                }
            }
            else
            {
                switch (depthStencilAttachment->texture_info->format)
                {
                    case internal::TextureFormat_TEXTUREFORMAT_D32_FLOAT_S8_UINT:
                    {
                        depthStencilAttachment->texture_info->format = internal::TextureFormat_TEXTUREFORMAT_D32_FLOAT;
                        break;
                    }
                    default:
                    {
                        depthStencilAttachment->texture_info->format = internal::TextureFormat_TEXTUREFORMAT_D24_UNORM;
                    }
                }
            }
        }



        bool fixedSize = depthStencilAttachment->texture_info->width == 0;
        if (ImGui::Checkbox("Fixed Size", &fixedSize))
        {
            if (fixedSize)
            {
                depthStencilAttachment->texture_info->width = 512;
                depthStencilAttachment->texture_info->height = 512;
            }
            else
            {
                depthStencilAttachment->texture_info->width = 0;
                depthStencilAttachment->texture_info->height = 0;
            }
        }
        if (fixedSize)
        {
            std::array targetSize{(int)depthStencilAttachment->texture_info->width, (int)depthStencilAttachment->texture_info->height};
            if (ImGui::InputInt2("Target Size", targetSize.data()))
            {
                depthStencilAttachment->texture_info->width = (targetSize[0]);
                depthStencilAttachment->texture_info->height = (targetSize[1]);
            }
        }
    }
    else
    {
        if(ImGui::Button("Add Depth/Stencil Attachment"))
        {
            auto depthStencilAttachment = std::make_unique<renderer::DepthStencilAttachmentT>();
            depthStencilAttachment->texture_info = std::make_unique<internal::TextureInfoT>();
            depthStencilAttachment->texture_info->format = internal::TextureFormat_TEXTUREFORMAT_D24_UNORM_S8_UINT;
            depthStencilAttachment->target_info = std::make_unique<internal::DepthStencilTargetInfoT>();

            currentFramebufferInfo.info.depth_stencil_attachment = std::move(depthStencilAttachment);
        }
    }
}

bool FramebufferEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = framebufferInfos_.size();
    for (std::size_t i = 0; i < framebufferInfos_.size(); i++)
    {
        const auto& shaderInfo = framebufferInfos_[i];
        if (ImGui::Selectable(shaderInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}

std::string_view FramebufferEditor::GetSubFolder()
{
    return "framebuffer/";
}

EditorType FramebufferEditor::GetEditorType()
{
    return EditorType::FRAMEBUFFER;
}

void FramebufferEditor::Save()
{
    for (auto& framebufferInfo : framebufferInfos_)
    {
        if (!core::WriteFlatbufferToFile<renderer::FramebufferT, renderer::Framebuffer>(framebufferInfo.info, framebufferInfo.path))
        {
            LogWarning(std::format("Could not save framebuffer at: {}", framebufferInfo.path.c_str()));
        }

    }
}

void FramebufferEditor::ReloadId()
{
}

void FramebufferEditor::Delete()
{
    if (currentIndex_ >= framebufferInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.RemoveResource(framebufferInfos_[currentIndex_].path, true);
}

std::span<const std::string_view> FramebufferEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = {
        ".framebuffer"
    };
    return extensions;
}

FramebufferInfo* FramebufferEditor::GetFramebuffer(ResourceId resourceId)
{
    const auto it = std::ranges::find_if(framebufferInfos_, [resourceId](const auto& framebufferInfo)
    {
        return framebufferInfo.resourceId == resourceId;
    });
    if(it != framebufferInfos_.end())
    {
        return &*it;
    }
    return nullptr;
}
void FramebufferEditor::Clear()
{
    framebufferInfos_.clear();
    currentIndex_ = -1;
}
}
