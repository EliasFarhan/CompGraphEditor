#pragma once
#include <array>

#include "engine/system.h"
#include "renderer/pipeline.h"
#include "gl/scene.h"
#include "engine/engine.h"

namespace gpr5300
{
    class SampleBrowserProgram : public core::System, public core::ImguiDrawInterface
    {
    public:
        void Begin() override;
        void Update(float dt) override;
        void End() override;
        void DrawImGui() override;

    private:
        gl::Scene scene_;
        core::SceneManager sceneManager_;
    };   
} // namespace gpr5300
