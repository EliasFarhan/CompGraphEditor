#include "vk/scene.h"

namespace gpr5300::vk
{
VkRenderPass GetCurrentRenderPass()
{
    auto* scene = static_cast<vk::Scene*>(GetCurrentScene());
    return scene->GetCurrentRenderPass();
}
}
