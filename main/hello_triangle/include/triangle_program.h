#pragma once
#include <array>

#include "engine/system.h"
#include "renderer/pipeline.h"

namespace gpr5300
{
    class HelloTriangleProgram : public System
    {
    public:
        void Begin() override;
        void Update(float dt) override;
        void End() override;

    private:
        std::array<Shader, 2> shaders_;
        Pipeline pipeline_;
    };   
} // namespace gpr5300
