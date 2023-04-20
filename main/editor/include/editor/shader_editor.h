#pragma once

#include "editor_system.h"
#include "proto/renderer.pb.h"

namespace editor
{

struct ShaderInfo
{
    std::string filename;
    core::pb::Shader info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
    bool compiledCorrectly = true;
};

class ShaderEditor final : public EditorSystem
{
public:
    void AddResource(const Resource &resource) override;
    void RemoveResource(const Resource &resource) override;
    void UpdateExistingResource(const Resource &resource) override;
    void DrawInspector() override;
    void DrawCenterView() override;
    std::string_view GetSubFolder() override;
    EditorType GetEditorType() override;
    bool DrawContentList(bool unfocus = false) override;
    void Save() override;
    const ShaderInfo* GetShader(ResourceId resourceId) const;
    [[nodiscard]] const auto& GetShaders() const { return shaderInfos_; }
    void ReloadId() override{}
    void Delete() override; std::span<const std::string_view> GetExtensions() const override;
    void Clear() override;
private:
    bool AnalyzeShader(std::string_view path, core::pb::Shader& shaderInfo) const;
    static core::pb::Attribute::Type GetType(std::string_view attributeTypeString);
    std::vector<ShaderInfo> shaderInfos_;
    std::size_t currentIndex_ = -1;
    std::string shaderText_;
};

}