#pragma once
#include <string>

#include "editor_system.h"
#include "resource.h"
#include "proto/renderer.pb.h"

namespace gpr5300
{

struct CommandInfo
{
    std::string filename;
    std::string path;
    pb::DrawCommand info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
    ResourceId materialId = INVALID_RESOURCE_ID;
    ResourceId meshId = INVALID_RESOURCE_ID;
};

class CommandEditor : public EditorSystem
{
public:
    void AddResource(const Resource& resource) override;
    void RemoveResource(const Resource& resource) override;
    void UpdateExistingResource(const Resource& resource) override;
    bool CheckExtensions(std::string_view extension) override;
    void DrawMainView() override;
    void DrawInspector() override;
    bool DrawContentList(bool unfocus) override;
    std::string_view GetSubFolder() override;
    EditorType GetEditorType() override;
    void Save() override;

private:
    std::vector<CommandInfo> commandInfos_;
    std::size_t currentIndex_ = -1;
};

} // namespace gpr5300
