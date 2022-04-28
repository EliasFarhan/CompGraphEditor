#pragma once
#include "editor_system.h"
#include "proto/renderer.pb.h"

namespace gpr5300
{
    struct ScriptInfo
    {
        std::string filename;
        ResourceId resourceId;
        pb::PySystem info;
        std::vector<std::string> classesInScript;

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
    const ScriptInfo* GetScriptInfo(ResourceId resourceId) const;
    const auto& GetScriptInfos() const { return scriptInfos_; }
    void Delete() override;
private:
    std::vector<ScriptInfo> scriptInfos_;
    std::size_t currentIndex_ = -1;
};
} // namespace grp5300
