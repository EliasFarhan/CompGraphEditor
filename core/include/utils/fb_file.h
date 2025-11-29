//
// Created by unite on 26.11.2025.
//

#ifndef NEKO2_FB_FILE_H
#define NEKO2_FB_FILE_H

#include "flatbuffers/flatbuffer_builder.h"
#include <flatbuffers/util.h>

#include "engine/filesystem.h"

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


    // Load the entire file into memory
    FileBuffer buffer = core::LoadFile(path);
    if (buffer.data == nullptr)
    {
        LogError(std::format("Failed to load file '{}'", path));
        return false;
    }

    // Verify buffer (optional but strongly recommended)
    flatbuffers::Verifier verifier(
        buffer.data,
        buffer.size
    );


    // Get the root
    const Object* table = flatbuffers::GetRoot<Object>(buffer.data);

    if (!table->Verify(verifier)) {
        LogError(std::format("Data is corrupt of wrong schema for '{}'", path));
        return false; // Data is corrupt or wrong schema
    }
    // Convert FlatBuffer → object-representation
    table->UnPackTo(&object);

    return true;
}
}

#endif // NEKO2_FB_FILE_H
