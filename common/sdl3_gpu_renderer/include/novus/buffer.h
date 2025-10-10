//
// Created by unite on 10.10.2025.
//

#ifndef NEKO2_BUFFER_H
#define NEKO2_BUFFER_H
#include "renderer/buffer.h"

namespace novus
{
class VertexInputBuffer : public core::VertexInputBuffer
{
public:
	void CreateFromMesh(const core::Mesh& mesh) override;

	void Bind() override;

	void Destroy() override;
};

class BufferManager: public core::BufferManager
{
public:
	core::BufferId CreateBuffer(std::string_view name, std::size_t count, std::size_t size) override;

	void Clear() override;

	core::BufferId GetBuffer(std::string_view bufferName) override;

	core::ArrayBuffer GetArrayBuffer(core::BufferId id) override;

	void CopyData(std::string_view bufferName, void* dataSrc, std::size_t length) override;

	void BindBuffer(core::BufferId id, int bindPoint) override;
};
}

#endif //NEKO2_BUFFER_H
