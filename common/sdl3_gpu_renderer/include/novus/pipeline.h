//
// Created by unite on 10.10.2025.
//

#ifndef NEKO2_PIPELINE_H
#define NEKO2_PIPELINE_H
#include "renderer/pipeline.h"

namespace novus
{
class Pipeline : public core::Pipeline
{
public:
	void Bind() override;
};
}
#endif //NEKO2_PIPELINE_H
