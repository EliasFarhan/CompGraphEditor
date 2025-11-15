#pragma once


#include "engine/system.h"
#include "engine/engine.h"
#include "renderer/camera.h"

#include <generated/renderer_generated.h>
#include <span>
#include <vector>

#include "utils/sdl_fb_impl.h"

namespace core
{
class ComputeCommand;
class Script;
class PyManager;
class DrawCommand;
class BufferManager;

class Scene;
class Framebuffer;
class Pipeline;
class Material;
class Mesh;

/**
 * \brief SceneMaterial is a class used by the python interface for scripting
 */
class SceneMaterial
{
    public:
        SceneMaterial(Pipeline* pipeline, Material* material);
        virtual ~SceneMaterial() = default;
        [[nodiscard]] Pipeline* GetPipeline() const;
        [[nodiscard]] Material* GetMaterial() const { return material_; }
        [[nodiscard]] std::string_view GetName() const;
    protected:
        Pipeline* pipeline_ = nullptr;
        Material* material_ = nullptr;
};



class SceneSubPass
{
public:
    SceneSubPass(Scene& scene, const novus::renderer::RenderpassT& subPass, int subPassIndex);
    [[nodiscard]] DrawCommand& GetDrawCommand(int drawCommandIndex) const;
    [[nodiscard]] int GetDrawCommandCount() const;
private:
    Scene& scene_;
    const novus::renderer::RenderpassT& subPass_;
    int subPassIndex_ = -1;
};


class Scene : public OnEventInterface
{
public:
    virtual ~Scene() = default;
    void LoadScene();
    virtual void UnloadScene() = 0;
    void SetScene(const novus::renderer::SceneT &scene);
    virtual void Update(float dt) = 0;
    virtual void Draw(DrawCommand& drawCommand, int instance = 1) = 0;
    virtual void Dispatch(ComputeCommand& command, int x, int y, int z) = 0;
    virtual Framebuffer& GetFramebuffer(int framebufferIndex) = 0;

    SceneSubPass GetSubpass(int subPassIndex);
    int GetSubpassCount() const;
    virtual SceneMaterial GetMaterial(int materialIndex) = 0;
    int GetMaterialCount() const;
    virtual Pipeline& GetPipeline(int index) = 0;
    int GetPipelineCount() const;
    virtual std::string_view GetMeshName(int index);
    int64_t GetMeshCount() const;

    Camera& GetCamera() { return camera_; }

    void OnEvent(SDL_Event& event) override;
    virtual DrawCommand& GetDrawCommand(int subPassIndex, int drawCommandIndex) = 0;

    virtual BufferManager& GetBufferManager() = 0;

    const novus::renderer::SceneT& GetInfo() const { return scene_; }
protected:
    enum class ImportStatus
    {
        SUCCESS,
        FAILURE
    };


    virtual ImportStatus LoadShaders(std::span<const novus::renderer::ShaderT> shadersPb) = 0;
    virtual ImportStatus LoadPipelines(std::span<const novus::renderer::GraphicsPipelineT> pipelines) = 0;
    //virtual ImportStatus LoadTextures(const PbRepeatField<pb::Texture>& textures) = 0;
    virtual ImportStatus LoadMaterials(std::span<const novus::renderer::MaterialT> materials) = 0;
    //virtual ImportStatus LoadModels(const PbRepeatField<std::string>& models) = 0;
    virtual ImportStatus LoadMeshes(std::span<const novus::renderer::MeshT> meshes) = 0;
    //virtual ImportStatus LoadFramebuffers(const PbRepeatField<pb::FrameBuffer>& framebuffers) = 0;
    //virtual ImportStatus LoadDrawCommands(std::span<const novus::renderer::DrawCommandT> commands) = 0;
    virtual ImportStatus LoadRenderPass(std::span<const novus::renderer::RenderpassT> renderPasses) = 0;
    virtual ImportStatus LoadBuffers(std::span<const novus::renderer::StorageBufferT> buffers) = 0;
    novus::renderer::SceneT scene_;
    std::vector<Script*> scripts_;
    
    Camera camera_;
};

class SceneManager : public System, public OnEventInterface
{
public:
    SceneManager();
    void LoadScene(Scene* scene);
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    [[nodiscard]] Scene* GetCurrentScene() const { return currentScene_; }
    void OnEvent(SDL_Event& event) override;
private:
    Scene* currentScene_ = nullptr;
};

Scene* GetCurrentScene();

} // namespace core
