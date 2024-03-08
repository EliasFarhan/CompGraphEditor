#include "framebuffer_editor.h"
#include "command_editor.h"
#include "scene_editor.h"
#include "model_editor.h"
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
#include <imnodes.h>

#include <array>
#include <fstream>



namespace editor
{
void RenderPassEditor::DrawInspector()
{
    if (currentIndex_ >= renderPassInfos_.size())
    {
        return;
    }
    const auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    auto* commandEditor = dynamic_cast<CommandEditor*>(editor->GetEditorSystem(EditorType::COMMAND));
    auto* modelEditor = dynamic_cast<ModelEditor*>(editor->GetEditorSystem(EditorType::MODEL));
    const auto* framebufferEditor = dynamic_cast<FramebufferEditor*>(editor->GetEditorSystem(EditorType::FRAMEBUFFER));
    const auto& commands = commandEditor->GetCommands();
    auto& currentRenderPass = renderPassInfos_[currentIndex_];
    int deleteSubpassIndex = -1;
    ImGui::Separator();
    for (int subpassIndex = 0; subpassIndex < currentRenderPass.info.sub_passes_size(); subpassIndex++)
    {
        const auto headerTitle = fmt::format("Subpass {}", subpassIndex);
        ImGui::PushID(headerTitle.c_str());
        ImGui::Text("%s", headerTitle.c_str());

        auto* subpassInfo = currentRenderPass.info.mutable_sub_passes(subpassIndex);
        std::array color
        {
            subpassInfo->subpass().clear_color().r(),
            subpassInfo->subpass().clear_color().g(),
            subpassInfo->subpass().clear_color().b(),
            subpassInfo->subpass().clear_color().a(),
        };
        if (ImGui::ColorEdit4("Clear Color", color.data()))
        {
            auto* clearColor = subpassInfo->mutable_subpass()->mutable_clear_color();
            clearColor->set_r(color[0]);
            clearColor->set_g(color[1]);
            clearColor->set_b(color[2]);
            clearColor->set_a(color[3]);
        }

        if(subpassInfo->subpass().has_viewport_size())
        {
            std::array viewport = {subpassInfo->subpass().viewport_size().x(), subpassInfo->subpass().viewport_size().y()};
            if(ImGui::InputInt2("Viewport Size", viewport.data()))
            {
                subpassInfo->mutable_subpass()->mutable_viewport_size()->set_x(viewport[0]);
                subpassInfo->mutable_subpass()->mutable_viewport_size()->set_y(viewport[1]);
            }
            if (ImGui::Button("Remove Viewport Size"))
            {
                subpassInfo->mutable_subpass()->clear_viewport_size();
            }
        }
        else
        {
            if(ImGui::Button("Add Viewport Size"))
            {
                auto* viewportSize = subpassInfo->mutable_subpass()->mutable_viewport_size();
            }
        }

        const auto& framebufferInfos = framebufferEditor->GetFramebuffers();
        const auto& framebufferPath = subpassInfo->framebuffer_path();
        if (ImGui::BeginCombo("Framebuffer", framebufferPath.empty() ? "Backbuffer" : framebufferPath.data()))
        {
            for (std::size_t framebufferIndex = 0; framebufferIndex < framebufferInfos.size(); framebufferIndex++)
            {
                if(ImGui::Selectable(framebufferInfos[framebufferIndex].filename.data(), 
                    framebufferPath.empty()?false:fs::equivalent(framebufferInfos[framebufferIndex].path, framebufferPath)))
                {
                    subpassInfo->set_framebuffer_path(framebufferInfos[framebufferIndex].path);
                }
            }
            ImGui::EndCombo();
        }
        if(ImGui::Button("Reset Framebuffer to Backbuffer"))
        {
            subpassInfo->clear_framebuffer_path();
        }

        std::vector<int> removedCommandIndices;
        for (int commandIndex = 0; commandIndex < subpassInfo->command_paths_size(); commandIndex++)
        {
            auto commandPath = subpassInfo->command_paths(commandIndex);
            const auto commandId = resourceManager.FindResourceByPath(commandPath);
            if (!commandPath.empty() && commandId == INVALID_RESOURCE_ID)
            {
                subpassInfo->mutable_command_paths(commandIndex)->clear();
                commandPath = "";
            }
            const auto* command = commandEditor->GetCommand(commandId);

            const auto commandHeaderTitle = fmt::format("Command {}", commandIndex);
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
                            subpassInfo->set_command_paths(commandIndex, availableCommand.path);
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopID();
            }
            ImGui::PopID();
            if (!visible)
            {
                removedCommandIndices.push_back(commandIndex);
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
        const auto importId = fmt::format("{}_import_model_command", headerTitle);
        const auto popupId = fmt::format("{}_popup", importId);
        bool openPopup = false;
        ImGui::PushID(importId.c_str());
        if (ImGui::Button("Import From Model Command"))
        {
            openPopup = true;
        }
        if(ImGui::Button("Remove Subpass"))
        {
            deleteSubpassIndex = subpassIndex;
        }
        ImGui::PopID();
        if (openPopup)
        {
            ImGui::OpenPopup(popupId.c_str());
        }
        if (ImGui::BeginPopup(popupId.c_str()))
        {
            const auto& models = modelEditor->GetModels();
            if (ImGui::BeginCombo("Model Command Selection", "Choose Command"))
            {
                for (auto& model : models)
                {
                    for (auto& command : model.drawCommands)
                    {
                        if (command.pipelineId == INVALID_RESOURCE_ID)
                            continue;
                        auto* pipelineResource = resourceManager.GetResource(command.pipelineId);
                        const auto commandName = fmt::format("{}_{}", GetFilename(model.path, false), GetFilename(pipelineResource->path, false));
                        if (ImGui::Selectable(commandName.c_str(), false))
                        {
                            //TODO import commands
                            for (auto commandId : command.drawCommandIds)
                            {
                                *subpassInfo->add_command_paths() = resourceManager.GetResource(commandId)->path;
                            }
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::EndPopup();
        }
        
        ImGui::PopID();
        ImGui::Separator();
    }
    if(deleteSubpassIndex != -1)
    {
        currentRenderPass.info.mutable_sub_passes()->DeleteSubrange(deleteSubpassIndex, 1);
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

void RenderPassEditor::DrawCenterView()
{
    if (currentIndex_ >= renderPassInfos_.size())
        return;
    const auto& currentRenderPassInfo = renderPassInfos_[currentIndex_];
    const auto* editor = Editor::GetInstance();
    auto* commandEditor = dynamic_cast<CommandEditor*>(editor->GetEditorSystem(EditorType::COMMAND));
    const auto& resourceManager = editor->GetResourceManager();
    ImNodes::BeginNodeEditor();
    const auto subpassCount = currentRenderPassInfo.info.sub_passes_size();
    constexpr auto inputAttribIndex = 100;
    constexpr auto outputAttribIndex = 200;
    std::vector<std::pair<int, int>> links;

    for(int i = 0; i < subpassCount; i++)
    {
        const auto& subpass = currentRenderPassInfo.info.sub_passes(i);
        ImNodes::BeginNode(i);

        ImNodes::BeginNodeTitleBar();
        ImGui::Text("Subpass %d", i);
        ImNodes::EndNodeTitleBar();

        if(i != 0)
        {
            ImNodes::BeginInputAttribute(inputAttribIndex+i);
            ImNodes::EndInputAttribute();
        }

        if (i != subpassCount - 1)
        {
            links.emplace_back(outputAttribIndex + i, inputAttribIndex + i + 1);
            ImNodes::BeginOutputAttribute(outputAttribIndex + i);
            ImNodes::EndOutputAttribute();
        }
        ImGui::TextUnformatted("Commands:");
        for(int commandIndex = 0; commandIndex < subpass.command_paths_size(); commandIndex++)
        {
            const auto& commandPath = subpass.command_paths(commandIndex);
            const auto commandResource = resourceManager.FindResourceByPath(commandPath);
            const auto& commandInfo = commandEditor->GetCommand(commandResource);
            if (commandInfo != nullptr)
            {
                if (commandInfo->info.index() == 0)
                {
                    ImGui::Text("%s", std::get<pb::EditorDrawCommand>(commandInfo->info).draw_command().name().c_str());
                }
                else
                {
                    ImGui::Text("%s", std::get<pb::EditorComputeCommand>(commandInfo->info).compute_command().name().c_str());
                }
            }
        }

        ImNodes::EndNode();

        ImNodes::SetNodeGridSpacePos(i, { 50+static_cast<float>(i)*150,150 });
    }
    for (std::size_t i = 0; i < links.size(); i++)
    {
        ImNodes::Link(i, links[i].first, links[i].second);
    }
    ImNodes::EndNodeEditor();
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

    const auto& fileSystem = core::FilesystemLocator::get();

    if (!fileSystem.IsRegularFile(resource.path.c_str()))
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
    resourceManager.RemoveResource(renderPassInfos_[currentIndex_].path, true);
}

std::span<const std::string_view> RenderPassEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = { ".r_pass" };
    return std::span{ extensions };
}

void RenderPassEditor::Clear()
{
    renderPassInfos_.clear();
    currentIndex_ = -1;
}
}
