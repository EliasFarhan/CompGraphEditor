#pragma once
#include <volk.h>

#include "engine/system.h"

namespace vk
{
class ImGuiManager
{
public:
    void Begin();
    void PreImGuiDraw();
    void PostImGuiDraw();
    void End() const;
private:
    void InitDescriptorPool();
    void UploadFontAtlas();

    VkDescriptorPool descriptorPool_;
};
}
