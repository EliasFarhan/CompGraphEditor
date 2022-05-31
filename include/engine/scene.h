#pragma once

#include "renderer/pipeline.h"
#include "engine/system.h"
#include "renderer/mesh.h"
#include "renderer/texture.h"
#include "py_interface.h"
#include "renderer/material.h"
#include "proto/renderer.pb.h"
#include "engine/engine.h"

#include <vector>

#include "renderer/framebuffer.h"
#include "renderer/model.h"

namespace gpr5300
{
class Scene;


class SceneMaterial
{
    public:
        SceneMaterial(Pipeline* pipeline, Material* material);
        void Bind() const;
        [[nodiscard]] Pipeline* GetPipeline() const;
        [[nodiscard]] std::string_view GetName() const;
    private:
        Pipeline* pipeline_ = nullptr;
        Material* material_ = nullptr;
};


class SceneDrawCommand
{
public:
    SceneDrawCommand(Scene& scene, const pb::DrawCommand& drawCommand);
    [[nodiscard]] SceneMaterial GetMaterial() const;
    void Draw();
    [[nodiscard]] std::string_view GetMeshName() const;
    [[nodiscard]] std::string_view GetName() const;
private:
    Scene& scene_;
    const pb::DrawCommand& drawCommand_;
};


class SceneSubPass
{
public:
    SceneSubPass(Scene& scene, const pb::SubPass& subPass);
    SceneDrawCommand GetDrawCommand(int drawCommandIndex) const;
    int GetDrawCommandCount() const;
    Framebuffer* GetFramebuffer();
private:
    Scene& scene_;
    const pb::SubPass& subPass_;
};



class Scene : public OnEventInterface
{
public:
    void LoadScene(PyManager& pyManager);
    void UnloadScene();
    void SetScene(const pb::Scene &scene);
    void Update(float dt);

    SceneSubPass GetSubpass(int subPassIndex);
    int GetSubpassCount() const;
    SceneMaterial GetMaterial(int materialIndex);
    int GetMaterialCount() const;
    Pipeline& GetPipeline(int index){ return pipelines_[index]; }
    int GetPipelineCount() const;
    Mesh& GetMesh(int index) { return meshes_[index]; }
    int GetMeshCount() const;
    void Draw(const pb::DrawCommand& drawCommand);
    void OnEvent(SDL_Event& event) override;
    Framebuffer& GetFramebuffer(int framebufferIndex);

private:
    pb::Scene scene_;
    std::vector<Shader> shaders_;
    std::vector<Pipeline> pipelines_;
    std::vector<Mesh> meshes_;
    std::vector<Model> models_;
    std::vector<Texture> textures_;
    std::vector<Material> materials_;
    std::vector<Script*> pySystems_;
    std::vector<Framebuffer> framebuffers_;
    //std::vector<SubPass> subpasses_;

};

class SceneManager : public System, public OnEventInterface
{
public:
    SceneManager();
    void LoadScene(Scene* scene);
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    Scene* GetCurrentScene() { return currentScene_; }
    static SceneManager* GetInstance(){ return sceneManager_; }
    TextureManager& GetTextureManager(){ return textureManager_; }
    void OnEvent(SDL_Event& event) override;
private:
    inline static SceneManager* sceneManager_ = nullptr;
    Scene* currentScene_ = nullptr;
    PyManager pyManager_;
    TextureManager textureManager_;
};

} // namespace gpr5300
