#include "vk/buffer.h"

namespace vk
{
void BufferManager::Clear()
{

}

core::BufferId BufferManager::CreateBuffer(std::string_view name, std::size_t count, std::size_t size)
{
    return {};
}

core::BufferId BufferManager::GetBuffer(std::string_view bufferName)
{
    return {};
}

core::ArrayBuffer BufferManager::GetArrayBuffer(core::BufferId id)
{
    return {};
}

void BufferManager::CopyData(std::string_view bufferName, void* dataSrc, std::size_t length)
{
}

void BufferManager::BindBuffer(core::BufferId id, int bindPoint)
{
}
}