#pragma once
#include "editor_system.h"
#include "proto/renderer.pb.h"

namespace gpr5300
{

struct FramebufferInfo
{
    std::string filename;
    ResourceId resourceId;
    pb::FrameBuffer info;
};

class FramebufferEditor : public EditorSystem
{
public:
    void AddResource(const Resource& resource) override;
    void RemoveResource(const Resource& resource) override;
    void UpdateExistingResource(const Resource& resource) override;
    void DrawMainView() override;
    void DrawInspector() override;
    bool DrawContentList(bool unfocus) override;
    std::string_view GetSubFolder() override;
    EditorType GetEditorType() override;
    void Save() override;
    void ReloadId() override;
    void Delete() override;
    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override;
private:
    std::vector<FramebufferInfo> framebufferInfos_;
};
}