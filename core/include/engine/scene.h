#pragma once


#include "engine/system.h"
#include "proto/renderer.pb.h"
#include "engine/engine.h"
#include "renderer/camera.h"

#include <span>
#include <vector>

namespace core
{
class Script;
class PyManager;
class DrawCommand;

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
    SceneSubPass(Scene& scene, const pb::SubPass& subPass, int subPassIndex);
    [[nodiscard]] DrawCommand& GetDrawCommand(int drawCommandIndex) const;
    [[nodiscard]] int GetDrawCommandCount() const;
    Framebuffer* GetFramebuffer();
private:
    Scene& scene_;
    const pb::SubPass& subPass_;
    int subPassIndex_ = -1;
};


class Scene : public OnEventInterface
{
public:
    void LoadScene();
    virtual void UnloadScene() = 0;
    void SetScene(const pb::Scene &scene);
    virtual void Update(float dt) = 0;
    virtual void Draw(DrawCommand& drawCommand) = 0;
    virtual Framebuffer& GetFramebuffer(int framebufferIndex) = 0;

    SceneSubPass GetSubpass(int subPassIndex);
    int GetSubpassCount() const;
    virtual SceneMaterial GetMaterial(int materialIndex) = 0;
    int GetMaterialCount() const;
    virtual Pipeline& GetPipeline(int index) = 0;
    int GetPipelineCount() const;
    virtual std::string_view GetMeshName(int index);
    int GetMeshCount() const;

    Camera& GetCamera() { return camera_; }

    void OnEvent(SDL_Event& event) override;
    virtual DrawCommand& GetDrawCommand(int subPassIndex, int drawCommandIndex) = 0;

    const pb::Scene& GetInfo() const { return scene_; }
protected:
    enum class ImportStatus
    {
        SUCCESS,
        FAILURE
    };

    template<typename T>
    using PbRepeatField = google::protobuf::RepeatedPtrField<T>;

    virtual ImportStatus LoadShaders(const PbRepeatField<pb::Shader> & shadersPb) = 0;
    virtual ImportStatus LoadPipelines(const PbRepeatField<pb::Pipeline>& pipelines) = 0;
    virtual ImportStatus LoadTextures(const PbRepeatField<pb::Texture>& textures) = 0;
    virtual ImportStatus LoadMaterials(const PbRepeatField<pb::Material>& materials) = 0;
    virtual ImportStatus LoadModels(const PbRepeatField<std::string>& models) = 0;
    virtual ImportStatus LoadMeshes(const PbRepeatField<pb::Mesh>& meshes) = 0;
    virtual ImportStatus LoadFramebuffers(const PbRepeatField<pb::FrameBuffer>& framebuffers) = 0;
    virtual ImportStatus LoadDrawCommands(const pb::RenderPass& renderPass) = 0;
    virtual ImportStatus LoadRenderPass(const pb::RenderPass& renderPass) = 0;

    pb::Scene scene_;
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
    Scene* GetCurrentScene() const { return currentScene_; }
    void OnEvent(SDL_Event& event) override;
private:
    Scene* currentScene_ = nullptr;
};

Scene* GetCurrentScene();

} // namespace core
