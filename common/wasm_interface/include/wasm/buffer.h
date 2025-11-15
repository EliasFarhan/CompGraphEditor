#pragma once


#include "wasm/neko2.h"
#include <ranges>

namespace script
{
class Buffer
{
public:
    Buffer(int64_t bufferId):bufferId_(bufferId){}

    template<std::ranges::contiguous_range R>
    void CopyData(const R& data) const
    {
        buffer_copy_data(bufferId_, std::ranges::data(data), std::ssize(data)*sizeof(std::ranges::range_value_t<R>));
    }
    template<typename T>
    void CopyData(const T& data, size_t offset) const
    {
        auto ptr = reinterpret_cast<const uint8_t*>(&data) + offset;
        buffer_copy_data(bufferId_, ptr, sizeof(T));
    }
private:
    int64_t bufferId_;
};

inline Buffer GetBuffer(std::string_view bufferName)
{
    return Buffer(get_buffer(bufferName.data()));
}
}
