#include "buffer_editor.h"

#include <fstream>

#include "command_editor.h"
#include "engine/filesystem.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <fmt/format.h>

namespace editor
{
void BufferEditor::AddResource(const Resource& resource)
{
    BufferInfo bufferInfo{};
    bufferInfo.resourceId = resource.resourceId;
    bufferInfo.filename = GetFilename(resource.path);

    const auto extension = GetFileExtension(resource.path);

    const auto& fileSystem = core::FilesystemLocator::get();
    if (!fileSystem.IsRegularFile(resource.path))
    {
        LogWarning(fmt::format("Could not find buffer file: {}", resource.path));
        return;
    }
    std::ifstream fileIn(resource.path, std::ios::binary);
    if (!bufferInfo.info.ParseFromIstream(&fileIn))
    {
        LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
        return;
    }

    bufferInfo.path = resource.path;
    buffersInfo_.push_back(bufferInfo);
}

void BufferEditor::RemoveResource(const Resource& resource)
{
    const auto it = std::ranges::find_if(buffersInfo_, [&resource](const auto& mesh)
        {
            return resource.resourceId == mesh.resourceId;
        });
    if (it != buffersInfo_.end())
    {
        buffersInfo_.erase(it);
        const auto* editor = Editor::GetInstance();

        auto* commandEditor = dynamic_cast<CommandEditor*>(editor->GetEditorSystem(EditorType::COMMAND));
        commandEditor->RemoveResource(resource);
    }
}

void BufferEditor::UpdateExistingResource(const Resource& resource)
{
}

void BufferEditor::DrawInspector()
{
    if (currentIndex_ >= buffersInfo_.size())
    {
        return;
    }
    auto& currentBuffer = buffersInfo_[currentIndex_];

    if(ImGui::InputText("Name",currentBuffer.info.mutable_name()))
    {
        
    }
    const auto currentType = currentBuffer.info.type();
    const auto& currentTypeString = core::pb::Attribute_Type_Name(currentType);
    if(ImGui::BeginCombo("Type", currentTypeString.data()))
    {
        for(int i = 0; i < core::pb::Attribute_Type_Type_ARRAYSIZE; i++)
        {
            if(!core::pb::Attribute_Type_IsValid(i))
            {
                continue;
            }
            const auto type = static_cast<core::pb::Attribute_Type>(i);
            const auto& typeString = core::pb::Attribute_Type_Name(type);
            if(ImGui::Selectable(typeString.data(),type == currentType))
            {
                currentBuffer.info.set_type(type);
            }
        }
        ImGui::EndCombo();
    }

    int count = currentBuffer.info.count();
    if(ImGui::InputInt("Count", &count))
    {
        currentBuffer.info.set_count(count);
    }
}

bool BufferEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = buffersInfo_.size();
    for (std::size_t i = 0; i < buffersInfo_.size(); i++)
    {
        const auto& bufferInfo = buffersInfo_[i];
        if (ImGui::Selectable(bufferInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}

std::string_view BufferEditor::GetSubFolder()
{
    return "buffers/";
}

EditorType BufferEditor::GetEditorType()
{
    return EditorType::BUFFER;
}

void BufferEditor::ReloadId()
{
}

void BufferEditor::Delete()
{

    if (currentIndex_ >= buffersInfo_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.RemoveResource(buffersInfo_[currentIndex_].path, true);
}

std::span<const std::string_view> BufferEditor::GetExtensions() const
{
    static std::array<std::string_view, 1> extensions = 
    {
        ".buffer"
    };
    return extensions;
}

void BufferEditor::Clear()
{
    buffersInfo_.clear();
    currentIndex_ = -1;
}

void BufferEditor::Save()
{
    for (auto& bufferInfo : buffersInfo_)
    {
        std::ofstream fileOut(bufferInfo.path, std::ios::binary);
        if (!bufferInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save buffer at: {}", bufferInfo.path));
        }

    }
}

BufferInfo* BufferEditor::GetBuffer(ResourceId resourceId)
{
    const auto it = std::ranges::find_if(buffersInfo_, [resourceId](const auto& meshInfo)
        {
            return resourceId == meshInfo.resourceId;
        });
    if (it != buffersInfo_.end())
    {
        return &*it;
    }
    return nullptr;
}
}
