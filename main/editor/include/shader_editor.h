#pragma once

#include "editor_system.h"

namespace gpr5300
{

struct ShaderInfo
{
    std::string filename;
    pb::Shader info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
};

class ShaderEditor : public EditorSystem
{
public:
    void AddResource(const Resource &resource) override;
    void RemoveResource(const Resource &resource) override;
    void UpdateResource(const Resource &resource) override;
    bool CheckExtensions(std::string_view extension) override;
    void DrawMainView() override;
    void DrawInspector() override;
    std::string_view GetSubFolder() override;
    EditorType GetEditorType() override;
    bool DrawContentList(bool unfocus = false) override;
private:
    bool AnalyzeShader(std::string_view path, pb::Shader& shaderInfo);
    static pb::Attribute::Type GetType(std::string_view attibuteTypeString);
    std::vector<ShaderInfo> shaderInfos_;
    std::size_t currentIndex_ = -1;
};

}