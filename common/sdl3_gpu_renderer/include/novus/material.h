
#ifndef NEKO2_MATERIAL_H
#define NEKO2_MATERIAL_H

#include <generated/renderer_generated.h>


#include "renderer/material.h"

namespace novus
{

class Material : public core::Material
{
public:
    std::string_view GetName() const override;
    void Load(const renderer::MaterialT& materialInfo);
    int GetPipelineIndex() const{return pipelineIndex_;}
private:
    std::string name_;
    int pipelineIndex_ = -1;
};

} // namespace novus

#endif // NEKO2_MATERIAL_H
