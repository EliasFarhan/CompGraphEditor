#include "framebuffer_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"
#include "editor.h"
#include <fmt/format.h>
#include <imgui_stdlib.h>
#include <array>
#include <fstream>

#include "gl/framebuffer.h"


namespace editor
{
void FramebufferEditor::AddResource(const Resource& resource)
{
    FramebufferInfo framebufferInfo{};
    framebufferInfo.filename = GetFilename(resource.path);
    framebufferInfo.resourceId = resource.resourceId;
    framebufferInfo.path = resource.path;

    const auto& fileSystem = core::FilesystemLocator::get();

    if (!fileSystem.IsRegularFile(resource.path))
    {
        LogWarning(fmt::format("Could not find framebuffer file: {}", resource.path));
        return;
    }
    std::ifstream fileIn(resource.path, std::ios::binary);
    if (!framebufferInfo.info.ParseFromIstream(&fileIn))
    {
        LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
        return;
    }
    if(framebufferInfo.info.name().empty())
    {
        framebufferInfo.info.set_name(GetFilename(resource.path, false));
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
    auto* framebufferName = currentFramebufferInfo.info.mutable_name();
    ImGui::InputText("Framebuffer Name", framebufferName);
    ImGui::Separator();
    int deletedColorAttachment = -1;
    for(int colorAttachmentIndex = 0; colorAttachmentIndex < currentFramebufferInfo.info.color_attachments_size(); colorAttachmentIndex++)
    {
        auto* colorAttachment = currentFramebufferInfo.info.mutable_color_attachments(colorAttachmentIndex);
        std::string id = fmt::format("Color Attachment {}", colorAttachmentIndex);
        std::string colorAttachmentName = colorAttachment->name().empty() ? id : colorAttachment->name();
        ImGui::PushID(id.data());
        if(ImGui::InputText("Color Attachment Name",&colorAttachmentName))
        {
            *colorAttachment->mutable_name() = colorAttachmentName;
        }
        static constexpr std::array<std::string_view, 4> formatTxt =
        {
            "RED",
            "RG",
            "RGB",
            "RGBA"
        };
        if(ImGui::BeginCombo("Format", formatTxt[colorAttachment->format()].data()))
        {
            for(std::size_t format = 0; format < formatTxt.size(); format++)
            {
                if(ImGui::Selectable(formatTxt[format].data(), format == colorAttachment->format()))
                {
                    colorAttachment->set_format(static_cast<core::pb::RenderTarget_Format>(format));
                }
            }
            ImGui::EndCombo();
        }
        static constexpr std::array<std::string_view, 4> sizeTxt =
        {
            "8",
            "16",
            "24",
            "32"
        };
        if(ImGui::BeginCombo("Size", sizeTxt[colorAttachment->format_size()].data()))
        {
            for(std::size_t sizeType = 0; sizeType < sizeTxt.size(); sizeType++)
            {
                if(ImGui::Selectable(sizeTxt[sizeType].data(), sizeType == colorAttachment->format_size()))
                {
                    colorAttachment->set_format_size(static_cast<core::pb::RenderTarget_FormatSize>(sizeType));
                }
            }
            ImGui::EndCombo();
        }
        static constexpr std::array<std::string_view, 3> typeTxt
        {
            "unsigned",
            "int",
            "float"
        };
        if(ImGui::BeginCombo("Type", typeTxt[colorAttachment->type()].data()))
        {
            for(std::size_t type = 0; type < typeTxt.size(); type++)
            {
                if(ImGui::Selectable(typeTxt[type].data(), type == colorAttachment->type()))
                {
                    colorAttachment->set_type(static_cast<core::pb::RenderTarget_Type>(type));
                }
            }
            ImGui::EndCombo();
        }

        bool fixedSize = colorAttachment->size_type() == core::pb::RenderTarget_Size_FIXED_SIZE;
        if(ImGui::Checkbox("Fixed Size", &fixedSize))
        {
            colorAttachment->set_size_type(fixedSize ? core::pb::RenderTarget_Size_FIXED_SIZE : core::pb::RenderTarget_Size_WINDOW_SIZE);
        }
        if(fixedSize)
        {
            std::array targetSize{colorAttachment->target_size().x(), colorAttachment->target_size().y()};
            if(ImGui::InputInt2("Target Size", targetSize.data()))
            {
                colorAttachment->mutable_target_size()->set_x(targetSize[0]);
                colorAttachment->mutable_target_size()->set_y(targetSize[1]);
            }
        }

        bool rbo = colorAttachment->rbo();
        if(ImGui::Checkbox("RBO", &rbo))
        {
            colorAttachment->set_rbo(rbo);
        }
        bool snorm = colorAttachment->snorm();
        if(ImGui::Checkbox("SNORM", &snorm))
        {
            colorAttachment->set_snorm(snorm);
        }
        const auto status = gl::GetAttachmentType(*colorAttachment);
        if(status.error != 0)
        {
            ImGui::TextColored({ 1,0,0,1 }, "Invalid attachment format");
        }

        if(ImGui::Button("Remove Color Attachment"))
        {
            deletedColorAttachment = colorAttachmentIndex;
        }
        
        ImGui::PopID();
        ImGui::Separator();
    }
    if(deletedColorAttachment != -1)
    {
        currentFramebufferInfo.info.mutable_color_attachments()->DeleteSubrange(deletedColorAttachment, 1);
    }
    if(ImGui::Button("Add Color Attachment"))
    {
        currentFramebufferInfo.info.add_color_attachments();
    }
    //depth/stencil attachment
    if(currentFramebufferInfo.info.has_depth_stencil_attachment())
    {
        auto* depthStencilAttachment = currentFramebufferInfo.info.mutable_depth_stencil_attachment();
        bool stencil = depthStencilAttachment->format() == core::pb::RenderTarget_Format_DEPTH_STENCIL;
        if(ImGui::Checkbox("Stencil", &stencil))
        {
            depthStencilAttachment->set_format(stencil ? core::pb::RenderTarget_Format_DEPTH_STENCIL : core::pb::RenderTarget_Format_DEPTH_COMP);
        }

        static constexpr std::array<std::string_view, 3> sizeTxt =
        {
            "16",
            "24",
            "32"
        };
        if (ImGui::BeginCombo("Size", sizeTxt[depthStencilAttachment->format_size()-1].data()))
        {
            for (std::size_t sizeType = 0; sizeType < sizeTxt.size(); sizeType++)
            {
                if (ImGui::Selectable(sizeTxt[sizeType].data(), sizeType+1 == depthStencilAttachment->format_size()))
                {
                    depthStencilAttachment->set_format_size(static_cast<core::pb::RenderTarget_FormatSize>(sizeType+1));
                    switch(depthStencilAttachment->format_size())
                    {
                    case core::pb::RenderTarget_FormatSize_SIZE_16:
                    case core::pb::RenderTarget_FormatSize_SIZE_24:
                        depthStencilAttachment->set_type(core::pb::RenderTarget_Type_UNSIGNED);
                        break;
                    case core::pb::RenderTarget_FormatSize_SIZE_32:
                        depthStencilAttachment->set_type(core::pb::RenderTarget_Type_FLOAT);
                        break;
                    default: 
                        break;
                    }
                }
            }
            ImGui::EndCombo();
        }
        bool rbo = depthStencilAttachment->rbo();
        if (ImGui::Checkbox("RBO", &rbo))
        {
            depthStencilAttachment->set_rbo(rbo);
        }


        const auto status = gl::GetAttachmentType(*depthStencilAttachment);
        if (status.error != 0)
        {
            ImGui::TextColored({ 1,0,0,1 }, "Invalid attachment format");
        }
        if(ImGui::Button("Remove Depth/Stencil Attachment"))
        {
            currentFramebufferInfo.info.clear_depth_stencil_attachment();
        }
    }
    else
    {
        if(ImGui::Button("Add Depth/Stencil Attachment"))
        {
            auto* depthStencilAttachment = currentFramebufferInfo.info.mutable_depth_stencil_attachment();
            depthStencilAttachment->set_format(core::pb::RenderTarget_Format_DEPTH_STENCIL);
            depthStencilAttachment->set_format_size(core::pb::RenderTarget_FormatSize_SIZE_24);
            depthStencilAttachment->set_type(core::pb::RenderTarget_Type_UNSIGNED);
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
        std::ofstream fileOut(framebufferInfo.path, std::ios::binary);
        if (!framebufferInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save framebuffer at: {}", framebufferInfo.path));
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
