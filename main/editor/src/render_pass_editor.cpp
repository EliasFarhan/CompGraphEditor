#include "render_pass_editor.h"
#include "proto/renderer.pb.h"
#include "resource.h"
#include "editor_system.h"
#include "engine/filesystem.h"
#include "utils/log.h"
#include "command_editor.h"
#include "editor.h"
#include <fmt/format.h>
#include <imgui.h>
#include <array>
#include <fstream>

#include "scene_editor.h"

namespace gpr5300
{


    bool RenderPassEditor::CheckExtensions(std::string_view extension)
    {
        return extension == ".r_pass";
    }

    void RenderPassEditor::DrawMainView() {

    }

    void RenderPassEditor::DrawInspector()
    {
        if (currentIndex_ >= renderPassInfos_.size())
        {
            return;
        }
        const auto* editor = Editor::GetInstance();
        const auto& resourceManager = editor->GetResourceManager();
        const auto* commandEditor = dynamic_cast<CommandEditor*>(editor->GetEditorSystem(EditorType::COMMAND));
        const auto& commands = commandEditor->GetCommands();
        auto& currentRenderPass = renderPassInfos_[currentIndex_];
        for (int i = 0; i < currentRenderPass.info.sub_passes_size(); i++)
        {
            bool headerVisible = true;
            const auto headerTitle = fmt::format("Subpass {}", i);
            if (ImGui::CollapsingHeader(headerTitle.c_str(), &headerVisible))
            {

                auto* subpassInfo = currentRenderPass.info.mutable_sub_passes(i);
                std::array color
                {
                    subpassInfo->clear_color().r(),
                    subpassInfo->clear_color().g(),
                    subpassInfo->clear_color().b(),
                    subpassInfo->clear_color().a(),
                };
                const auto clearColorId = fmt::format("Clear Color Subpass {}", i);
                ImGui::PushID(clearColorId.c_str());
                if (ImGui::ColorEdit4("Clear Color", color.data()))
                {
                    auto* clearColor = subpassInfo->mutable_clear_color();
                    clearColor->set_r(color[0]);
                    clearColor->set_g(color[1]);
                    clearColor->set_b(color[2]);
                    clearColor->set_a(color[3]);
                }
                ImGui::PopID();
                std::vector<int> removedCommandIndices;
                for (int j = 0; j < subpassInfo->command_paths_size(); j++)
                {
                    auto commandPath = subpassInfo->command_paths(j);
                    const auto commandId = resourceManager.FindResourceByPath(commandPath);
                    if(!commandPath.empty() && commandId == INVALID_RESOURCE_ID)
                    {
                        subpassInfo->mutable_command_paths(j)->clear();
                        commandPath = "";
                    }
                    const auto* command = commandEditor->GetCommand(commandId);

                    const auto commandHeaderTitle = fmt::format("Command {}", j);
                    bool visible = true;
                    const auto commandHeaderId = fmt::format("{}{}", headerTitle, commandHeaderTitle);
                    ImGui::PushID(commandHeaderId.c_str());
                    if (ImGui::CollapsingHeader(commandHeaderTitle.c_str(), &visible))
                    {
                        const auto comboId = commandHeaderId + "combo";
                        ImGui::PushID(comboId.c_str());
                        if (ImGui::BeginCombo("Command", command ? command->filename.c_str() : "Empty command"))
                        {
                            for (auto& availableCommand : commands)
                            {
                                if (ImGui::Selectable(availableCommand.filename.c_str(), availableCommand.resourceId == commandId))
                                {
                                    subpassInfo->set_command_paths(j, availableCommand.path);
                                }
                            }
                            ImGui::EndCombo();
                        }
                        ImGui::PopID();
                    }
                    ImGui::PopID();
                    if (!visible)
                    {
                        removedCommandIndices.push_back(j);
                    }
                }
                std::ranges::sort(removedCommandIndices);
                std::ranges::reverse(removedCommandIndices);
                for (const auto removedCommandIndex : removedCommandIndices)
                {
                    subpassInfo->mutable_command_paths()->DeleteSubrange(removedCommandIndex, 1);
                }
                const auto buttonId = fmt::format("{}_add_command_button", headerTitle);
                ImGui::PushID(buttonId.c_str());
                if (ImGui::Button("Add Command"))
                {
                    subpassInfo->add_command_paths();
                }
                ImGui::PopID();
            }
            if (!headerVisible)
            {
                currentRenderPass.info.mutable_sub_passes()->DeleteSubrange(i, 1);
            }
        }
        if (ImGui::Button("Add SubPass"))
        {
            currentRenderPass.info.add_sub_passes();
        }
    }

    bool RenderPassEditor::DrawContentList(bool unfocus)
    {
        bool wasFocused = false;
        if (unfocus)
            currentIndex_ = renderPassInfos_.size();
        for (std::size_t i = 0; i < renderPassInfos_.size(); i++)
        {
            const auto& shaderInfo = renderPassInfos_[i];
            if (ImGui::Selectable(shaderInfo.filename.data(), currentIndex_ == i))
            {
                currentIndex_ = i;
                wasFocused = true;
            }
        }
        return wasFocused;
    }

    std::string_view RenderPassEditor::GetSubFolder()
    {
        return "render_passes/";
    }

    EditorType RenderPassEditor::GetEditorType()
    {
        return EditorType::RENDER_PASS;
    }

    void RenderPassEditor::Save()
    {
        for (auto& renderPassInfo : renderPassInfos_)
        {
            std::ofstream fileOut(renderPassInfo.path, std::ios::binary);
            if (!renderPassInfo.info.SerializeToOstream(&fileOut))
            {
                LogWarning(fmt::format("Could not save render pass at: {}", renderPassInfo.path));
            }
        }
    }

    void RenderPassEditor::AddResource(const Resource& resource)
    {
        RenderPassInfo renderPassInfo{};
        renderPassInfo.path = resource.path;
        renderPassInfo.resourceId = resource.resourceId;
        renderPassInfo.filename = GetFilename(resource.path);

        const auto& fileSystem = FilesystemLocator::get();

        if (!fileSystem.IsRegularFile(resource.path))
        {
            LogWarning(fmt::format("Could not find render pass file: {}", resource.path));
            return;
        }
        std::ifstream fileIn(resource.path, std::ios::binary);
        if (!renderPassInfo.info.ParsePartialFromIstream(&fileIn))
        {
            LogWarning(fmt::format("Could not open render pass protobuf file: {}", resource.path));
            return;
        }
        renderPassInfos_.push_back(renderPassInfo);
    }

    void RenderPassEditor::RemoveResource(const Resource& resource)
    {
        const auto it = std::ranges::find_if(renderPassInfos_, [&resource](const auto& renderPass)
            {
                return resource.resourceId == renderPass.resourceId;
            });
        if (it != renderPassInfos_.end())
        {
            renderPassInfos_.erase(it);
            const auto* editor = Editor::GetInstance();
            auto* sceneEditor = dynamic_cast<SceneEditor*>(editor->GetEditorSystem(EditorType::SCENE));

            sceneEditor->RemoveResource(resource);
        }
    }

    void RenderPassEditor::UpdateExistingResource(const Resource& resource)
    {

    }

    const RenderPassInfo* RenderPassEditor::GetRenderPass(ResourceId resourceId) const
    {
        const auto it = std::ranges::find_if(renderPassInfos_, [resourceId](const auto& renderPass)
            {
                return resourceId == renderPass.resourceId;
            });
        if (it != renderPassInfos_.end())
        {
            return &*it;
        }
        return nullptr;
    }

    void RenderPassEditor::ReloadId()
    {

    }

    void RenderPassEditor::Delete()
    {
        if (currentIndex_ >= renderPassInfos_.size())
        {
            return;
        }
        auto* editor = Editor::GetInstance();
        auto& resourceManager = editor->GetResourceManager();
        resourceManager.RemoveResource(renderPassInfos_[currentIndex_].path);
    }
}
