#pragma once
#include <array>

#include "engine/system.h"
#include "renderer/pipeline.h"
#include "gl/scene.h"
#include "engine/engine.h"

namespace gpr5300
{
    class HelloTriangleProgram : public System, public ImguiDrawInterface
    {
    public:
        void Begin() override;
        void Update(float dt) override;
        void End() override;
        void DrawImGui() override;

    private:
        gl::Scene scene_;
        SceneManager sceneManager_;
    };   
} // namespace gpr5300
