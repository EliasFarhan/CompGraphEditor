#include "vk/imgui_manager.h"

#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>

#include "utils/log.h"
#include "vk/engine.h"

namespace gpr5300::vk
{


void ImGuiManager::Begin()
{
    LogDebug("Init ImGui Manager");
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.KeysDown[ImGuiKey_Delete] = SDL_GetScancodeFromKey(SDLK_DELETE);
    auto& window = GetWindow();
    ImGui_ImplSDL2_InitForVulkan(window.GetWindow());


    InitDescriptorPool();

    const auto& driver = window.GetDriver();
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

    auto& renderer = GetRenderer();
    const auto& renderPass = renderer.GetCurrentDrawingProgram().renderPass;

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
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderer.commandBuffers[renderer.currentFrame]);
}

void ImGuiManager::End()
{
}

void ImGuiManager::InitDescriptorPool()
{
    auto& window = GetWindow();
    auto& driver = window.GetDriver();
    // Create Descriptor Pool

    VkDescriptorPoolSize pool_sizes[] =
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
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
    pool_info.pPoolSizes = pool_sizes;
    if (vkCreateDescriptorPool(driver.device, &pool_info, nullptr, &descriptorPool_) != VK_SUCCESS)
    {
        LogError("Could create descriptor pool for ImGui");
        std::terminate();
    }
}

void ImGuiManager::UploadFontAtlas()
{
    LogDebug("Upload ImGui Font Atlas");
    auto& window = GetWindow();
    auto& driver = window.GetDriver();
    auto& renderer = GetRenderer();
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
