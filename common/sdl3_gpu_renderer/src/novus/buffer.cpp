//
// Created by unite on 10.10.2025.
//
#include "novus/buffer.h"

namespace novus
{
void VertexInputBuffer::CreateFromMesh(const core::Mesh& mesh)
{
}

void VertexInputBuffer::Bind()
{
}

void VertexInputBuffer::Destroy()
{
}

core::BufferId BufferManager::CreateBuffer(std::string_view name, std::size_t count, std::size_t size)
{
}

void BufferManager::Clear()
{
}

core::BufferId BufferManager::GetBuffer(std::string_view bufferName)
{
}

core::ArrayBuffer BufferManager::GetArrayBuffer(core::BufferId id)
{
}

void BufferManager::CopyData(std::string_view bufferName, void* dataSrc, std::size_t length)
{
}

void BufferManager::BindBuffer(core::BufferId id, int bindPoint)
{
}
}
