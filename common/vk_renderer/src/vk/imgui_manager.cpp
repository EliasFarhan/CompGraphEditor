#include "vk/imgui_manager.h"

#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#include "utils/log.h"
#include "vk/engine.h"
#include "vk/scene.h"

namespace vk
{


void ImGuiManager::Begin()
{
    LogDebug("Init ImGui Manager");
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    auto& window = GetWindow();
    ImGui_ImplSDL2_InitForVulkan(window.GetSdlWindow());


    InitDescriptorPool();

    auto& driver = window.GetDriver();
    const auto& swapchain = window.GetSwapChain();
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = driver.instance;
    initInfo.Device = driver.device;
    initInfo.PhysicalDevice = driver.physicalDevice;
    initInfo.Queue = driver.graphicsQueue;
    initInfo.QueueFamily = VK_QUEUE_GRAPHICS_BIT;
    initInfo.DescriptorPool = descriptorPool_;
    initInfo.MinImageCount = swapchain.minImageCount;
    initInfo.ImageCount = swapchain.imageCount;
    initInfo.Subpass = 0;
    
    const auto& renderPass = GetCurrentRenderPass();
    ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* vulkan_instance) {
        return vkGetInstanceProcAddr(*(reinterpret_cast<VkInstance*>(vulkan_instance)), function_name);
        }, &driver.instance);
    ImGui_ImplVulkan_Init(&initInfo, renderPass);

    UploadFontAtlas();
}

void ImGuiManager::PreImGuiDraw()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::PostImGuiDraw()
{
    auto& renderer = GetRenderer();
    ImGui::Render();
    //ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderer.commandBuffers[renderer.imageIndex]);
}

void ImGuiManager::End() const
{
    const auto& driver = GetDriver();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    vkDestroyDescriptorPool(driver.device, descriptorPool_, nullptr);

    ImGui::DestroyContext();
}

void ImGuiManager::InitDescriptorPool()
{
    const auto& driver = GetDriver();
    // Create Descriptor Pool

    constexpr VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
    poolInfo.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(poolSizes));
    poolInfo.pPoolSizes = poolSizes;
    if (vkCreateDescriptorPool(driver.device, &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS)
    {
        LogError("Could create descriptor pool for ImGui");
        std::terminate();
    }
}

void ImGuiManager::UploadFontAtlas()
{
    LogDebug("Upload ImGui Font Atlas");
    const auto& driver = GetDriver();
    const auto& renderer = GetRenderer();
    auto& engine = GetEngine();

    // Use any command queue

    vkResetCommandPool(driver.device, renderer.commandPool, 0);


    const auto commandBuffer = engine.BeginSingleTimeCommands();
    {
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    }
    engine.EndSingleTimeCommands(commandBuffer);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}
}
