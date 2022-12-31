#pragma once
#include <vulkan/vulkan.h>

#include "engine/system.h"

namespace gpr5300::vk
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
