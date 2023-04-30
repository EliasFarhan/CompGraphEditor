#pragma once

#include "editor_system.h"
#include "pipeline_editor.h"
#include "proto/editor.pb.h"

namespace editor
{

struct MaterialInfo
{
    std::string path;
    std::string filename;
    pb::EditorMaterial info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
    ResourceId pipelineId = INVALID_RESOURCE_ID;
};

class MaterialEditor final : public EditorSystem
{
public:

    void DrawInspector() override;

    bool DrawContentList(bool unfocus) override;

    void DrawCenterView() override;

    std::string_view GetSubFolder() override;

    EditorType GetEditorType() override;

    void Save() override;

    void AddResource(const Resource &resource) override;

    void RemoveResource(const Resource &resource) override;

    void UpdateExistingResource(const Resource &resource) override;
    MaterialInfo* GetMaterial(ResourceId resourceId);
    const auto& GetMaterials() const { return materialInfos_; }
    void ReloadId() override;
    void Delete() override;

    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override;

    void Clear() override;
private:
    void ReloadMaterialPipeline(const PipelineInfo& pipelineInfo, int materialIndex);
    std::vector<MaterialInfo> materialInfos_;
    std::size_t currentIndex_ = -1;
};
}