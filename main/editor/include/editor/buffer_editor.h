#pragma once
#include "editor.h"
#include "proto/renderer.pb.h"

namespace editor
{

struct BufferInfo
{
    core::Path path;
    std::string filename;
    core::pb::Buffer info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
};

class BufferEditor final : public EditorSystem
{
public:
    void AddResource(const Resource& resource) override;
    void RemoveResource(const Resource& resource) override;
    void UpdateExistingResource(const Resource& resource) override;
    void DrawInspector() override;
    bool DrawContentList(bool unfocus) override;
    std::string_view GetSubFolder() override;
    EditorType GetEditorType() override;
    void ReloadId() override;
    void Delete() override;
    [[nodiscard]] std::span<const std::string_view> GetExtensions() const override;
    void Clear() override;
    void Save() override;
    BufferInfo* GetBuffer(ResourceId resourceId);
    std::span<BufferInfo> GetBuffers() { return buffersInfo_; }

private:
    std::vector<BufferInfo> buffersInfo_;
    std::size_t currentIndex_ = -1;
};

}
