#pragma once

#include <span>

#include "engine/system.h"
#include "renderer/texture.h"
#include "py_interface.h"
#include "proto/renderer.pb.h"
#include "engine/engine.h"


#include <vector>

namespace gpr5300
{

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
        /**
         * \brief Bind is a method that bind the underlying pipeline and material to the draw command
         */
        virtual void Bind() const = 0;
        [[nodiscard]] Pipeline* GetPipeline() const;
        [[nodiscard]] std::string_view GetName() const;
    protected:
        Pipeline* pipeline_ = nullptr;
        Material* material_ = nullptr;
};


class SceneDrawCommand
{
public:
    SceneDrawCommand(Scene& scene, const pb::DrawCommand& drawCommand);
    [[nodiscard]] std::unique_ptr<SceneMaterial> GetMaterial() const;
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
    virtual void UnloadScene() = 0;
    void SetScene(const pb::Scene &scene);
    virtual void Update(float dt) = 0;
    virtual void Draw(const pb::DrawCommand& drawCommand) = 0;
    virtual Framebuffer& GetFramebuffer(int framebufferIndex) = 0;

    SceneSubPass GetSubpass(int subPassIndex);
    int GetSubpassCount() const;
    virtual std::unique_ptr<SceneMaterial> GetMaterial(int materialIndex) = 0;
    int GetMaterialCount() const;
    virtual Pipeline& GetPipeline(int index) = 0;
    int GetPipelineCount() const;
    virtual Mesh& GetMesh(int index) = 0;
    int GetMeshCount() const;

    void OnEvent(SDL_Event& event) override;
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

    virtual ImportStatus LoadRenderPass(const pb::RenderPass& renderPass) { return ImportStatus::SUCCESS; }

    pb::Scene scene_;
    std::vector<Script*> pySystems_;
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
    PyManager pyManager_;
};

Scene* GetCurrentScene();

} // namespace gpr5300
