#pragma once

#include "editor_system.h"
#include "pipeline_editor.h"
#include "proto/renderer.pb.h"

namespace gpr5300
{

struct MaterialInfo
{
    std::string path;
    std::string filename;
    pb::Material info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
    ResourceId pipelineId = INVALID_RESOURCE_ID;
};

class MaterialEditor : public EditorSystem
{
public:
    bool CheckExtensions(std::string_view extension) override;

    void DrawMainView() override;

    void DrawInspector() override;

    bool DrawContentList(bool unfocus) override;

    std::string_view GetSubFolder() override;

    EditorType GetEditorType() override;

    void Save() override;

    void AddResource(const Resource &resource) override;

    void RemoveResource(const Resource &resource) override;

    void UpdateExistingResource(const Resource &resource) override;
    const MaterialInfo* GetMaterial(ResourceId resourceId) const;
    const auto& GetMaterials() const { return materialInfos_; }
    void ReloadId() override;
    void Delete() override;
private:
    void ReloadMaterialPipeline(const PipelineInfo& pipelineInfo, int materialIndex);
    std::vector<MaterialInfo> materialInfos_;
    std::size_t currentIndex_ = -1;
};
}