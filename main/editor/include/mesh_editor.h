#pragma once

namespace gpr5300
{

struct MeshInfo
{
    std::string path;
    std::string filename;
    pb::Mesh info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
};

class MeshEditor : public EditorSystem
{
public:
    bool CheckExtensions(std::string_view modelExtension) override;

    void DrawMainView() override;

    void DrawInspector() override;

    bool DrawContentList(bool unfocus) override;

    std::string_view GetSubFolder() override;

    EditorType GetEditorType() override;

    void Save() override;

    void AddResource(const Resource &resource) override;

    void RemoveResource(const Resource &resource) override;

    void UpdateResource(const Resource &resource) override;
private:
    std::vector<MeshInfo> meshInfos_;
    std::size_t currentIndex_ = -1;
};

}