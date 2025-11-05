//
// Created by unite on 05.11.2025.
//

#include "novus/material.h"

namespace novus
{
std::string_view Material::GetName() const
{
    return name_;
}
void Material::Load(const renderer::MaterialT& materialInfo)
{
    name_ = materialInfo.name;
    pipelineIndex_ = materialInfo.pipeline_index;
}
}