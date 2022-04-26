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
    pb::Scene info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
    ResourceId renderPassId = INVALID_RESOURCE_ID;
};

class SceneEditor : public EditorSystem
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
    bool ExportScene();
private:
    std::vector<SceneInfo> sceneInfos_;
    std::size_t currentIndex_ = -1;
};
}