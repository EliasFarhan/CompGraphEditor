#pragma once

#include <vector>
#include "renderer/pipeline.h"

namespace gpr5300
{
    class Scene
    {
        public:

        private:
            std::vector<Shader> shaders_;
            std::vector<Pipeline> pipelines_;
            //std::vector<SubPass> subpasses_;

    };

    class SceneManager : public System
    {
        public:

    };

} // namespace gpr5300
