#pragma once

#include "proto/renderer.pb.h"
#include "editor_system.h"

namespace gpr5300
{

struct SceneInfo
{
    std::string filename;
    std::string path;
    pb::Scene scene;
    ResourceId resourceId = INVALID_RESOURCE_ID;
};

class SceneEditor : public EditorSystem
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

};

}