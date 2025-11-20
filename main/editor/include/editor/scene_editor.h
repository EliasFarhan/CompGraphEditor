#pragma once

#include "resource.h"
#include "editor_system.h"
#include "generated/editor_generated.h"

namespace novus::editor
{

struct SceneInfo
{
    std::string filename;
    std::string path;
    EditorSceneInfo info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
    ResourceId renderPassId = INVALID_RESOURCE_ID;
};

class SceneEditor final : public EditorSystem
{
public:
    void ImportResource(std::string_view path) override;
    void AddResource(const Resource& resource) override;
    void RemoveResource(const Resource& resource) override;
    void UpdateExistingResource(const Resource& resource) override;
    void DrawInspector() override;
    bool DrawContentList(bool unfocus) override;
    std::string_view GetSubFolder() override;
    EditorType GetEditorType() override;
    void Save() override;
    bool ExportAndPlayScene() const;
    void ReloadId() override;
    void Delete() override;
    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override;
    EditorSceneInfoT* GetCurrentSceneInfo() { return currentIndex_ >= sceneInfos_.size() ? nullptr : &sceneInfos_[currentIndex_]; }
    void SetCurrentScene() { currentIndex_ = 0; }

    void Clear() override;
    bool IsVulkanScene() const;

private:
    std::vector<EditorSceneInfoT> sceneInfos_;
    std::size_t currentIndex_ = -1;
};

SceneEditor* GetSceneEditor();
}