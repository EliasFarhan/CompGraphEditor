#pragma once

#include "editor_system.h"
#include "resource.h"
#include "proto/editor.pb.h"

#include <variant>
#include <string>

namespace editor
{

struct CommandInfo
{
    std::string filename;
    core::Path path;
    std::variant<pb::EditorDrawCommand, pb::EditorComputeCommand> info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
    ResourceId materialId = INVALID_RESOURCE_ID;
    ResourceId meshId = INVALID_RESOURCE_ID;
    ResourceId bufferId = INVALID_RESOURCE_ID;
};

class CommandEditor final : public EditorSystem
{
public:
    void AddResource(const Resource& resource) override;
    void RemoveResource(const Resource& resource) override;
    void UpdateExistingResource(const Resource& resource) override;
    void DrawInspector() override;
    bool DrawContentList(bool unfocus) override;
    std::string_view GetSubFolder() override;
    EditorType GetEditorType() override;
    void Save() override;

    const auto& GetCommands() const { return commandInfos_; }
    CommandInfo* GetCommand(ResourceId resourceId);
    void ReloadId() override;
    void Delete() override;
    void UpdateMeshInCommand(int index);
    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override;
    void Clear() override;
private:

    std::vector<CommandInfo> commandInfos_;
    std::size_t currentIndex_ = -1;

};

} // namespace gpr5300
