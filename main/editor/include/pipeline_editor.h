#pragma once
#include "editor_system.h"
#include "proto/renderer.pb.h"

namespace gpr5300
{

struct PipelineInfo
{
    std::string path;
    std::string filename;
    pb::Pipeline info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
    ResourceId vertexShaderId = INVALID_RESOURCE_ID;
    ResourceId fragmentShaderId = INVALID_RESOURCE_ID;
};

class PipelineEditor final : public EditorSystem
{
public:
    void DrawMainView() override;
    void DrawInspector() override;
    bool DrawContentList(bool unfocus = false) override;
    std::string_view GetSubFolder() override;
    EditorType GetEditorType() override;
    void AddResource(const Resource& resource) override;
    void RemoveResource(const Resource& resource) override;
    void UpdateExistingResource(const Resource& resource) override;
    void Save() override;
    const PipelineInfo* GetPipeline(ResourceId resourceId) const;
    const auto& GetPipelines() const{ return pipelineInfos_;}
    void ReloadId() override;
    void Delete() override;
    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override;
private:
    void ReloadPipeline(int index);
    std::vector<PipelineInfo> pipelineInfos_;
    std::size_t currentIndex_ = -1;
};

}