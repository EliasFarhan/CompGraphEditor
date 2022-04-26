#pragma once

namespace gpr5300
{


struct RenderPassInfo
{
    std::string path;
    std::string filename;
    pb::RenderPass info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
};

class RenderPassEditor : public EditorSystem
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

    void UpdateResource(const Resource &resource) override;

};
}