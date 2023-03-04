#include "renderer/pipeline.h"

namespace core
{


void Pipeline::SetPipelineName(std::string_view name)
{
    pipelineName_ = name;
}

std::string_view Pipeline::GetPipelineName() const
{
    return pipelineName_;
}

} // namespace core
