#include "vk/common.h"

#include "utils/log.h"
#include <fmt/core.h>

namespace vk
{
bool CheckError(VkResult result)
{
    switch (result)
    {
    case VK_SUCCESS:
        return false;
    case VK_NOT_READY:
        LogError("[Vulkan] Not ready");
        return true;
    case VK_TIMEOUT:
        LogError("[Vulkan] Timeout");
        return true;
    case VK_EVENT_SET: break;
    case VK_EVENT_RESET: break;
    case VK_INCOMPLETE:
        LogError("[Vulkan] Incomplete");
        return true;
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        LogError("[Vulkan] Out of host memory");
        return true;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        LogError("[Vulkan] Out of device memory");
        return true;
    case VK_ERROR_INITIALIZATION_FAILED:
        LogError("[Vulkan] Initialization failed");
        return true;
    case VK_ERROR_DEVICE_LOST:
        LogError("[Vulkan] Device lost");
        return true;
    case VK_ERROR_MEMORY_MAP_FAILED:
        LogError("[Vulkan] Memory map failed");
        return true;
    case VK_ERROR_LAYER_NOT_PRESENT: break;
    case VK_ERROR_EXTENSION_NOT_PRESENT: break;
    case VK_ERROR_FEATURE_NOT_PRESENT: break;
    case VK_ERROR_INCOMPATIBLE_DRIVER: break;
    case VK_ERROR_TOO_MANY_OBJECTS: break;
    case VK_ERROR_FORMAT_NOT_SUPPORTED: break;
    case VK_ERROR_FRAGMENTED_POOL: break;
    case VK_ERROR_UNKNOWN:
        LogError("[Vulkan] Error unknown");
        return true;
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        LogError("[Vulkan] Out of pool memory");
        return true;
    case VK_ERROR_INVALID_EXTERNAL_HANDLE: break;
    case VK_ERROR_FRAGMENTATION: break;
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: break;
    case VK_ERROR_SURFACE_LOST_KHR: break;
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: break;
    case VK_SUBOPTIMAL_KHR: break;
    case VK_ERROR_OUT_OF_DATE_KHR: break;
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: break;
    case VK_ERROR_VALIDATION_FAILED_EXT: break;
    case VK_ERROR_INVALID_SHADER_NV: break;
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: break;
    case VK_ERROR_NOT_PERMITTED_EXT: break;
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: break;
    case VK_THREAD_IDLE_KHR: break;
    case VK_THREAD_DONE_KHR: break;
    case VK_OPERATION_DEFERRED_KHR: break;
    case VK_OPERATION_NOT_DEFERRED_KHR: break;
    case VK_PIPELINE_COMPILE_REQUIRED_EXT: break;
    case VK_RESULT_MAX_ENUM: break;
    default:;
    }
    LogError(fmt::format("[Vulkan] Error Code: {}", result));
    return true;
}
}
