#pragma once

#include <tiny_obj_loader.h>

#include "editor_system.h"
#include "proto/renderer.pb.h"

namespace gpr5300
{
    

struct ModelInfo
{
    std::string path;
    std::string filename;
    pb::Model info;
    tinyobj::ObjReader reader;
    ResourceId resourceId = INVALID_RESOURCE_ID;
};

class ModelEditor final : public EditorSystem
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
    ModelInfo* GetModel(ResourceId resourceId);
    void ImportResource(std::string_view path) override;
private:
    std::vector<ModelInfo> modelInfos_;
    std::size_t currentIndex_ = -1;
};

}