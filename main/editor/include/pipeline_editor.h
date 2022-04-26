#pragma once

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

class PipelineEditor : public EditorSystem
{
public:
    bool CheckExtensions(std::string_view extension) override;
    void DrawMainView() override;
    void DrawInspector() override;
    bool DrawContentList(bool unfocus = false) override;
    std::string_view GetSubFolder() override;
    EditorType GetEditorType() override;
    void AddResource(const Resource& resource) override;
    void RemoveResource(const Resource& resource) override;
    void UpdateResource(const Resource& resource) override;
    void Save() override;
private:
    std::vector<PipelineInfo> pipelineInfos_;
    std::size_t currentIndex_ = -1;
};

}