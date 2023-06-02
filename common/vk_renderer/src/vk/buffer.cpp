#include "vk/buffer.h"

namespace vk
{
void BufferManager::Clear()
{

}

core::BufferId BufferManager::CreateBuffer(std::size_t count, std::size_t size)
{
    return core::BufferId();
}

void* BufferManager::GetArrayBuffer(core::BufferId id)
{
    return nullptr;
}
}