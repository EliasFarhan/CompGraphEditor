#pragma once

#include "resource.h"
#include "proto/renderer.pb.h"
#include "editor_system.h"

namespace gpr5300
{

struct SceneInfo
{
    std::string filename;
    std::string path;
    core::pb::Scene info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
    ResourceId renderPassId = INVALID_RESOURCE_ID;
};

class SceneEditor final : public EditorSystem
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
    bool ExportScene() const;
    void ReloadId() override;
    void Delete() override;
    std::span<const std::string_view> GetExtensions() const override;
private:
    std::vector<SceneInfo> sceneInfos_;
    std::size_t currentIndex_ = -1;
};
}