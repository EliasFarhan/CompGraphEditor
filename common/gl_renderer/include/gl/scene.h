#pragma once

#include <engine/scene.h>
#include <vector>

namespace gpr5300::gl
{
    
class Scene : public gpr5300::Scene
{
public: 
private:
    std::vector<Shader> shaders_;
    std::vector<Pipeline> pipelines_;
    std::vector<Mesh> meshes_;
    std::vector<Model> models_;
    std::vector<Texture> textures_;
    std::vector<Material> materials_;
    std::vector<Framebuffer> framebuffers_;
};
} // namespace gpr5300::gl
