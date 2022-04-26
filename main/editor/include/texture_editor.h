#pragma once
#include "renderer/texture.h"
namespace gpr5300
{

struct TextureInfo
{
    std::string path;
    std::string filename;
    pb::Texture info;
    ResourceId resourceId = INVALID_RESOURCE_ID;
};

class TextureEditor : public EditorSystem
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
private:
    std::vector<TextureInfo> textureInfos_;
};
}