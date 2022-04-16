#pragma once
#include <array>

#include "engine/system.h"
#include "renderer/pipeline.h"
#include "engine/scene.h"

namespace gpr5300
{
    class HelloTriangleProgram : public System
    {
    public:
        void Begin() override;
        void Update(float dt) override;
        void End() override;

    private:
        Scene scene_;
        SceneManager sceneManager_;
    };   
} // namespace gpr5300
