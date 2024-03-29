#pragma once

#include "resource.h"
#include "editor_system.h"
#include "proto/editor.pb.h"
#include "proto/renderer.pb.h"

namespace editor
{

struct MeshInfo
{
    core::Path path;
    std::string filename;
    editor::pb::EditorMesh info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
};

class MeshEditor final : public EditorSystem
{
public:

    void DrawInspector() override;

    bool DrawContentList(bool unfocus) override;

    std::string_view GetSubFolder() override;

    EditorType GetEditorType() override;

    void Save() override;

    void AddResource(const Resource &resource) override;

    void RemoveResource(const Resource &resource) override;

    void UpdateExistingResource(const Resource &resource) override;
    const auto& GetMeshes() const { return meshInfos_; }
    MeshInfo* GetMesh(ResourceId resourceId);
    void ReloadId() override{}
    void Delete() override;
    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override;
    void Clear() override;
private:
    std::vector<MeshInfo> meshInfos_;
    std::size_t currentIndex_ = -1;
};

}