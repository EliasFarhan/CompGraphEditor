#pragma once

#include <string_view>

#include "editor_system.h"
#include "resource.h"
#include "proto/renderer.pb.h"

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

    void UpdateExistingResource(const Resource &resource) override;
    const auto& GetRenderPasses() const { return renderPassInfos_; }
    const RenderPassInfo* GetRenderPass(ResourceId resourceId) const;
    void ReloadId() override;
private:
    std::vector<RenderPassInfo> renderPassInfos_;
    std::size_t currentIndex_ = -1;
};
}
