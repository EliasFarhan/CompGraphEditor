#pragma once
#include "editor_system.h"

namespace gpr5300
{
    struct ScriptInfo
    {
        std::string path;
        std::string className;
        ResourceId resourceId;

    };

class ScriptEditor : public EditorSystem
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
    void ReloadId() override;
private:
    std::vector<ScriptInfo> scriptInfos_;
};
} // namespace grp5300
