#pragma once
#include <string_view>
#include "resource.h"

namespace gpr5300
{

enum class EditorType
{
    SHADER,
    PIPELINE,
    TEXTURE,
    MODEL,
    MATERIAL,
    LENGTH
};

class EditorSystem : public ResourceChangeInterface
{
public:
    virtual ~EditorSystem() = default;
    virtual bool CheckExtensions(std::string_view extension) = 0;
    virtual void DrawMainView() = 0;
    virtual void DrawInspector() = 0;
    virtual std::string_view GetSubFolder() = 0;
    virtual EditorType GetEditorType() = 0;
};
}