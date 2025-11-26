//
// Created by unite on 26.11.2025.
//

#ifndef NEKO2_FB_FILE_H
#define NEKO2_FB_FILE_H

#include "flatbuffers/flatbuffer_builder.h"
#include <flatbuffers/util.h>

namespace core
{

template <typename ObjectT, typename Object>
bool WriteFlatbufferToFile(const ObjectT& object, std::string_view path)
{
    flatbuffers::FlatBufferBuilder builder;
    auto rootOffset = Object::Pack(builder, &object);
    builder.Finish(rootOffset);
    return flatbuffers::SaveFile(path.data(), reinterpret_cast<const char*>(builder.GetBufferPointer()),
        builder.GetSize(), true);
}
template <typename ObjectT, typename Object>
bool ReadFlatbufferFromFile(std::string_view path, ObjectT& object) {

    std::string buffer;

    // Load the entire file into memory
    bool ok = flatbuffers::LoadFile(path.data(), /*binary=*/true, &buffer);
    if (!ok) {
        return false;
    }

    // Verify buffer (optional but strongly recommended)
    flatbuffers::Verifier verifier(
        reinterpret_cast<const uint8_t*>(buffer.data()),
        buffer.size()
    );

    if (!Object::Verify(verifier)) {
        return false; // Data is corrupt or wrong schema
    }

    // Get the root
    const Object* table = flatbuffers::GetRoot<Object>(buffer.data());

    // Convert FlatBuffer → object-representation
    table->UnPackTo(&object);

    return true;
}
}

#endif // NEKO2_FB_FILE_H
