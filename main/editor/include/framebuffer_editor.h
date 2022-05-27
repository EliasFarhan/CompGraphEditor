#pragma once
#include "editor_system.h"
#include "proto/renderer.pb.h"

namespace gpr5300
{

struct FramebufferInfo
{
    std::string filename;
    std::string path;
    pb::FrameBuffer info;
    ResourceId resourceId;
};

class FramebufferEditor : public EditorSystem
{
public:
    void AddResource(const Resource& resource) override;
    void RemoveResource(const Resource& resource) override;
    void UpdateExistingResource(const Resource& resource) override;
    void DrawInspector() override;
    bool DrawContentList(bool unfocus) override;
    std::string_view GetSubFolder() override;
    EditorType GetEditorType() override;
    void Save() override;
    void ReloadId() override;
    void Delete() override;
    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override;

    const auto& GetFramebuffers() const { return framebufferInfos_; }
    FramebufferInfo* GetFramebuffer(ResourceId resourceId);
private:
    std::vector<FramebufferInfo> framebufferInfos_;
    std::size_t currentIndex_ = -1;
};
}
