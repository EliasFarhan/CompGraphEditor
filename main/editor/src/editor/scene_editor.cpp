#include "scene_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"
#include "render_pass_editor.h"
#include "command_editor.h"

#include <imgui.h>
#include <format>
#include <fstream>

#include "editor.h"


#include <pybind11/pybind11.h>
#include <nlohmann/json.hpp>

#include "buffer_editor.h"
#include "framebuffer_editor.h"
#include "material_editor.h"
#include "mesh_editor.h"
#include "model_editor.h"
#include "pipeline_editor.h"
#include "script_editor.h"
#include "shader_editor.h"
#include "texture_editor.h"
#include "utils/fb_file.h"

namespace py = pybind11;
using json = nlohmann::json;

namespace novus::editor
{

void ExecutePlayer(std::string_view scenePkg)
{
    std::string baseName = "gpu_player";
    std::string command; 
#ifdef _MSC_VER
    fs::path executable = std::format("{}.exe", baseName);
    if (fs::exists(executable))
    {
        command = std::format("{} {}", executable.generic_string(), scenePkg);
    }
    else
    {
#ifdef _DEBUG
        command = std::format(".\\Debug\\{} {}", executable.generic_string(), scenePkg);
#else
        command = std::format(".\\Release\\{} {}", executable.generic_string(), scenePkg);
#endif
    }
#else
    command = std::format("./{} {}", baseName, scenePkg);
#endif

    if(command.empty())
    {
        return;
    }
    system(command.c_str());
    LogDebug(std::format("Executing {}", command));
}

void SceneEditor::ImportResource(std::string_view path)
{
    //TODO Only copy scene if in another folder
    
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.AddResource(path);
    
}

void SceneEditor::AddResource(const Resource& resource)
{

    if (!CheckExtensions(resource.extension))
    {
        //Adding a resource to the list
        const auto* sceneInfo = GetCurrentSceneInfo();
        if (std::ranges::none_of(sceneInfo->info.resources, [&resource](const auto& path)
            {
                return path == resource.path;
            }))
        {
            if (core::FileExists(resource.path))
            {
                GetCurrentSceneInfo()->info.resources.push_back(resource.path);
            }
        }
        return;
    }
    SceneInfo sceneInfo{};
    sceneInfo.resourceId = resource.resourceId;
    sceneInfo.filename = GetFilename(resource.path);

    const auto extension = GetFileExtension(resource.path);
    if (extension == ".scene")
    {
        if (!core::IsRegularFile(resource.path))
        {
            LogWarning(std::format("Could not find scene file: {}", resource.path.c_str()));
            return;
        }
        std::ifstream fileIn(resource.path.c_str(), std::ios::binary);
        if (!core::ReadFlatbufferFromFile<EditorSceneInfoT, EditorSceneInfo>(resource.path, sceneInfo.info))
        {
            LogWarning(std::format("Could not open protobuf file: {}", resource.path.c_str()));
            return;
        }
    }
    sceneInfo.path = resource.path;
    sceneInfos_.push_back(sceneInfo);

    currentIndex_ = 0;

    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    std::vector<int> unexistingResources;
    
    for(int i = 0; i < sceneInfo.info.resources.size(); i++)
    {
        std::string_view resourcePath {sceneInfo.info.resources[i]};
        if(!core::FileExists(resourcePath))
        {
            unexistingResources.push_back(i);
            continue;
        }
        if (resourceManager.FindResourceByPath(resourcePath) == INVALID_RESOURCE_ID)
        {
            resourceManager.AddResource(resourcePath);
        }

    }
    auto& resourcesPb = GetCurrentSceneInfo()->info.resources;
    for(int i = static_cast<int>(unexistingResources.size()) - 1; i >= 0; i--)
    {
        LogWarning(std::format("Removing file {} from scene: file does not exists", sceneInfo.info.resources[unexistingResources[i]]));
        resourcesPb.erase(resourcesPb.begin()+unexistingResources[i]);
    }
    core::SetWindowName(std::format("Neko2 Editor - {}", sceneInfo.info.name));
}

void SceneEditor::RemoveResource(const Resource& resource)
{
    if(!CheckExtensions(resource.extension))
    {
        //Removing a resource to the list
        auto& resources = GetCurrentSceneInfo()->info.resources;
        for(size_t i = 0; i < resources.size(); i++)
        {
            if(resources[i] == resource.path)
            {
                resources.erase(resources.begin() + i);
                return;
            }
        }
        return;
    }
    for(auto& sceneInfo : sceneInfos_)
    {
        if(resource.resourceId == sceneInfo.renderPassId)
        {
            sceneInfo.renderPassId = INVALID_RESOURCE_ID;
            sceneInfo.info.renderpass_path.clear();
        }
    }

    const auto it = std::ranges::find_if(sceneInfos_, [&resource](const auto& renderPass)
        {
            return resource.resourceId == renderPass.resourceId;
        });
    if (it != sceneInfos_.end())
    {
        sceneInfos_.erase(it);
    }
}

void SceneEditor::UpdateExistingResource(const Resource& resource)
{
}


void SceneEditor::DrawInspector()
{
    if (currentIndex_ >= sceneInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    auto* renderPassEditor = dynamic_cast<RenderPassEditor*>(editor->GetEditorSystem(EditorType::RENDER_PASS));
    auto* scriptEditor = dynamic_cast<ScriptEditor*>(editor->GetEditorSystem(EditorType::SCRIPT));
    auto* modelEditor = dynamic_cast<ModelEditor*>(editor->GetEditorSystem(EditorType::MODEL));
    auto& currentScene = sceneInfos_[currentIndex_];


    const auto& renderPasses = renderPassEditor->GetRenderPasses();
    const auto* renderPassInfo = renderPassEditor->GetRenderPass(currentScene.renderPassId);
    if(ImGui::BeginCombo("Render Pass", renderPassInfo?renderPassInfo->filename.c_str():"Empty Render Pass"))
    {
        for(auto& renderPass : renderPasses)
        {
            if(ImGui::Selectable(renderPass.filename.c_str(), renderPass.resourceId == currentScene.renderPassId))
            {
                currentScene.renderPassId = renderPass.resourceId;
                currentScene.info.renderpass_path = (renderPass.path);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::Separator();
    ImGui::Text("Scripts");
    
    std::vector<int> removedScripts;
    for (size_t i = 0; i < currentScene.info.system_paths.size(); i++)
    {
        std::string_view pySystemPath {currentScene.info.system_paths[i]};
        std::string name = std::format("Script: {}", i);
        const ScriptInfo* scriptInfo = nullptr;
        ResourceId pySystemId = INVALID_RESOURCE_ID;
        if (!pySystemPath.empty())
        {
            if (!std::ranges::any_of(core::GetNativeScriptClassNames(), [&pySystemPath](const auto nativeScript)
                {
                return nativeScript == pySystemPath; }))
            {
                pySystemId = resourceManager.FindResourceByPath(pySystemPath);
                if (pySystemId == INVALID_RESOURCE_ID)
                {
                    currentScene.info.system_paths[i].clear();
                }
                else
                {
                    scriptInfo = scriptEditor->GetScriptInfo(pySystemId);
                    name = std::format("Script: {}.{}", scriptInfo->info.module_, scriptInfo->info.class_);
                }
            }
            else
            {
                name = std::format("Script: {}.{}", core::nativeModuleName, pySystemPath.data());
            }
        }
        bool visible = true;
        if (ImGui::CollapsingHeader(name.c_str(), &visible))
        {
            const auto& wasmScripts = scriptEditor->GetScriptInfos();
            const auto scriptsId = std::format("script {} combo", i);
            ImGui::PushID(scriptsId.c_str());
            if (ImGui::BeginCombo("Available Scripts", scriptInfo ? 
                scriptInfo->info.class_.c_str() :
                currentScene.info.system_paths[i].empty()?"Missing script": currentScene.info.system_paths[i].c_str()))
            {
                for (auto& wasmSystemInfo : wasmScripts)
                {
                    const auto selectedName = std::format("{}.{}", 
                        wasmSystemInfo.info.module_,
                        wasmSystemInfo.info.class_);
                    if (ImGui::Selectable(selectedName.c_str(), pySystemId == wasmSystemInfo.resourceId))
                    {
                        currentScene.info.system_paths[i] = wasmSystemInfo.info.path;
                    }
                }
                for(auto nativeScript: core::GetNativeScriptClassNames())
                {
                    const auto selectedName = std::format("{}.{}", core::nativeModuleName, nativeScript);
                    if(ImGui::Selectable(selectedName.c_str(), currentScene.info.system_paths[i] == nativeScript))
                    {
                        currentScene.info.system_paths[i] = nativeScript;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopID();
            
        }
        if (!visible)
        {
            removedScripts.push_back(i);
        }
        
    }
    std::ranges::reverse(removedScripts);
    for(const auto index : removedScripts)
    {
        currentScene.info.system_paths.erase(currentScene.info.system_paths.begin()+index);
    }
    if(ImGui::Button("Add Script"))
    {
        currentScene.info.system_paths.push_back({});
    }
    if (ImGui::BeginListBox("Editor Scene Resources"))
    {
        for (size_t i = 0; i < currentScene.info.resources.size(); i++)
        {
            auto& resource = currentScene.info.resources[i];
            ImGui::Selectable(resource.c_str(), false);
        }
        ImGui::EndListBox();
    }
}

bool SceneEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    for (std::size_t i = 0; i < sceneInfos_.size(); i++)
    {
        const auto& sceneInfo = sceneInfos_[i];
        const auto selected = ImGui::Selectable(sceneInfo.filename.data(), !unfocus);

        if (selected)
        {

            currentIndex_ = i;
            wasFocused = true;

        }
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::Button("Export Scene & Play"))
            {
                currentIndex_ = i;
                wasFocused = true;
                if(ExportAndPlayScene())
                {
                    LogDebug("Successfully export scene!");
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    return wasFocused;
}

std::string_view SceneEditor::GetSubFolder()
{
    //No subfolder for scenes, as each scene has their own data folder
    return "";
}

EditorType SceneEditor::GetEditorType()
{
    return EditorType::SCENE;
}

void SceneEditor::Save()
{
    for (auto& sceneInfo : sceneInfos_)
    {
        std::ofstream fileOut(sceneInfo.path.c_str(), std::ios::binary);
        if (!core::WriteFlatbufferToFile<EditorSceneInfoT, EditorSceneInfo>(sceneInfo.info, sceneInfo.path))
        {
            LogWarning(std::format("Could not save scene at: {}", sceneInfo.path.c_str()));
        }
    }
}

bool SceneEditor::ExportAndPlayScene() const
{
    if (currentIndex_ >= sceneInfos_.size())
    {
        LogWarning("Could not export no scene selected");
        return false;
    }
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    const auto* renderPassEditor = dynamic_cast<RenderPassEditor*>(editor->GetEditorSystem(EditorType::RENDER_PASS));
    auto* commandEditor = dynamic_cast<CommandEditor*>(editor->GetEditorSystem(EditorType::COMMAND));
    auto* materialEditor = dynamic_cast<MaterialEditor*>(editor->GetEditorSystem(EditorType::MATERIAL));
    const auto* pipelineEditor = dynamic_cast<PipelineEditor*>(editor->GetEditorSystem(EditorType::PIPELINE));
    const auto* shaderEditor = dynamic_cast<ShaderEditor*>(editor->GetEditorSystem(EditorType::SHADER));
    auto* meshEditor = dynamic_cast<MeshEditor*>(editor->GetEditorSystem(EditorType::MESH));
    auto* modelEditor = dynamic_cast<ModelEditor*>(editor->GetEditorSystem(EditorType::MODEL));
    const auto* scriptEditor = dynamic_cast<ScriptEditor*>(editor->GetEditorSystem(EditorType::SCRIPT));
    auto* textureEditor = dynamic_cast<TextureEditor*>(editor->GetEditorSystem(EditorType::TEXTURE));
    auto* framebufferEditor = dynamic_cast<FramebufferEditor*>(editor->GetEditorSystem(EditorType::FRAMEBUFFER));
    auto* bufferEditor = dynamic_cast<BufferEditor*>(editor->GetEditorSystem(EditorType::BUFFER));
    //TODO reload all editors to get all correct resourceId
    commandEditor->ReloadId();
    const auto& currentScene = sceneInfos_[currentIndex_];
    novus::renderer::SceneT exportScene;
    //auto exportingScene = sceneInfos_[currentIndex_];
    const auto pkgSceneName = currentScene.path + ".pkg";
    //Validate scene
    
    if(currentScene.renderPassId == INVALID_RESOURCE_ID)
    {
        LogWarning("Could not export scene, missing render pass");
        return false;
    }
    std::unordered_map<ResourceId, int> resourceIndexMap;
    std::vector<std::string> cubemapTextures;
    auto* currentRenderPass = renderPassEditor->GetRenderPass(currentScene.renderPassId);
    //*exportRenderPass = currentRenderPass->info;
    //TODO export scene

    for(int subPassIndex = 0; subPassIndex < currentRenderPass->info.subpass_infos.size(); subPassIndex++)
    {
        const auto& editorSubPass = currentRenderPass->info.subpass_infos[subPassIndex];
        renderer::RenderpassT exportSubPass{};


        //export framebuffer
        const std::string_view framebufferPath {editorSubPass.framebuffer_path};
        if(!framebufferPath.empty())
        {
            const auto framebufferId = resourceManager.FindResourceByPath(framebufferPath);
            if(framebufferId == INVALID_RESOURCE_ID)
            {
                LogWarning(std::format("Could not export scene, invalid resource id for framebuffer. Framebuffer path: {}", framebufferPath.data()));
                return false;
            }
            auto* framebufferInfo = framebufferEditor->GetFramebuffer(framebufferId);
            auto framebufferIt = resourceIndexMap.find(framebufferInfo->resourceId);
            if (framebufferIt == resourceIndexMap.end())
            {
                auto index = exportScene.framebuffers.size();
                renderer::FramebufferT newFramebuffer{};
                newFramebuffer = framebufferInfo->info;
                exportSubPass.framebuffer_index = (index);
                resourceIndexMap[framebufferId] = index;
                exportScene.framebuffers.push_back(std::move(newFramebuffer));
            }
            else
            {
                exportSubPass.framebuffer_index = (framebufferIt->second);
            }
        }
        else
        {
            exportSubPass.framebuffer_index= -1; // setting back to backbuffer
        }
        //Export commands
        for(int commandIndex = 0; commandIndex < editorSubPass.command_paths.size(); commandIndex++)
        {
            const std::string_view editorCommandPath {editorSubPass.command_paths[commandIndex]};
            const auto commandId = resourceManager.FindResourceByPath(editorCommandPath);
            if(commandId == INVALID_RESOURCE_ID)
            {
                LogWarning(std::format("Could not export scene, missing command in subpass. Command path: {}", editorCommandPath.data()));
                return false;
            }
            const auto* editorCommand = commandEditor->GetCommand(commandId);

            std::optional<renderer::DrawCommandT> exportDrawCommand = std::nullopt;
            //renderer::ComputeCommandT* exportComputeCommand = nullptr;
            if (editorCommand->info.index() == 0)
            {
                exportDrawCommand = {};
                exportDrawCommand = *std::get<EditorDrawCommandInfoT>(editorCommand->info).draw_command;
            }
            //TODO add compute command
            /*
            else
            {
                exportComputeCommand = exportSubPass->add_compute_commands();
                *exportComputeCommand = std::get<pb::EditorComputeCommand>(editorCommand->info).compute_command();
            }
            */
            if (editorCommand->materialId == INVALID_RESOURCE_ID)
            {
                LogWarning(std::format("Could not export scene, missing material in command. Command {}", exportDrawCommand->name));
                return false;
            }
            int exportedPipelineIndex = -1;
            //link material index
            const auto* material = materialEditor->GetMaterial(editorCommand->materialId);
            auto materialIndexIt = resourceIndexMap.find(material->resourceId);
            if (materialIndexIt == resourceIndexMap.end())
            {
                const auto materialIndex = exportScene.materials.size();
                renderer::MaterialT exportMaterial{};
                resourceIndexMap[material->resourceId] = materialIndex;

                //list textures
                for (int textureIndex = 0; textureIndex < material->info.textures.size(); textureIndex++)
                {
                    const auto& editorMaterialTexture = material->info.textures[textureIndex];
                    renderer::TextureMaterialBindingT exportMaterialTextureBinding{};
                    //TODO attach correct texture binding to material
                    /*
                    if (editorMaterialTexture.path.empty() &&
                        editorMaterialTexture.material_texture().attachment_name().empty())
                    {
                        LogWarning(std::format(
                            "Could not export, missing texture/attachment in material sampler. Material: {} Sampler: {}",
                            material->info.material().name(), editorMaterialTexture.material_texture().sampler_name()));
                        exportMaterialTextureBinding->set_texture_index(-1);
                        return false;
                    }

                    if (editorMaterialTexture.material_texture().attachment_name().empty())
                    {
                        auto textureId = resourceManager.FindResourceByPath(editorMaterialTexture.texture_name());
                        if (textureId == INVALID_RESOURCE_ID)
                        {
                            LogWarning(std::format("Could not export scene. Missing texture in material sampler, Material: {} Sampler: {}",
                                material->info.material().name(), editorMaterialTexture.material_texture().sampler_name()));
                            exportMaterialTextureBinding->set_texture_index(-1);
                            return false;
                        }
                        auto textureIt = resourceIndexMap.find(textureId);
                        if (textureIt == resourceIndexMap.end())
                        {
                            const auto index = exportScene.textures_size();
                            auto* textureInfo = textureEditor->GetTexture(textureId);
                            *exportScene.add_textures() = textureInfo->info;
                            if (GetFileExtension(textureInfo->info.path()) == ".cube")
                            {
                                for (auto& cubeTexture : textureInfo->cubemap.texture_paths())
                                {
                                    if (!cubeTexture.empty())
                                    {
                                        cubemapTextures.push_back(cubeTexture);
                                    }
                                }
                            }
                            resourceIndexMap[textureId] = index;
                            exportMaterialTextureBinding->set_texture_index(index);
                        }
                        else
                        {
                            exportMaterialTextureBinding->set_texture_index(textureIt->second);
                        }
                        exportMaterialTextureBinding->set_sampler_name(editorMaterialTexture.material_texture().sampler_name());
                    }
                    else //framebuffer attachment for sampler
                    {
                        bool isValid = false;
                        for (const auto& framebufferPb : exportScene.framebuffers())
                        {
                            const auto& framebufferName = framebufferPb.name();
                            const auto& materialSamplerFramebufferName = editorMaterialTexture.material_texture().framebuffer_name();
                            if (framebufferName == materialSamplerFramebufferName)
                            {
                                for (const auto& colorAttachment : framebufferPb.color_attachments())
                                {
                                    if (!colorAttachment.rbo() && colorAttachment.name() == editorMaterialTexture.material_texture().attachment_name())
                                    {
                                        exportMaterialTextureBinding->set_attachment_name(editorMaterialTexture.material_texture().attachment_name());
                                        exportMaterialTextureBinding->set_framebuffer_name(editorMaterialTexture.material_texture().framebuffer_name());
                                        isValid = true;
                                        exportMaterialTextureBinding->set_sampler_name(editorMaterialTexture.material_texture().sampler_name());
                                    }
                                }
                                if (framebufferPb.has_depth_stencil_attachment() && !framebufferPb.depth_stencil_attachment().rbo())
                                {
                                    if (framebufferPb.depth_stencil_attachment().name() == editorMaterialTexture.material_texture().attachment_name())
                                    {
                                        exportMaterialTextureBinding->set_attachment_name(editorMaterialTexture.material_texture().attachment_name());
                                        exportMaterialTextureBinding->set_framebuffer_name(editorMaterialTexture.material_texture().framebuffer_name());
                                        isValid = true;
                                        exportMaterialTextureBinding->set_sampler_name(editorMaterialTexture.material_texture().sampler_name());
                                    }
                                }
                            }
                        }
                        if (!isValid)
                        {
                            LogWarning(std::format("Could not export, invalid framebuffer attachment in material sampler. Material: {} Sampler: {}",
                                material->info.material().name(), editorMaterialTexture.material_texture().sampler_name()));
                            return false;
                        }
                        exportMaterialTextureBinding->set_texture_index(-1);
                    }
                    */
                }

                //check if pipeline exists
                if (material->pipelineId == INVALID_RESOURCE_ID)
                {
                    LogWarning(std::format("Could not export scene, missing pipeline in material. Material: {}", material->path));
                    return false;
                }
                const auto* pipeline = pipelineEditor->GetPipeline(material->pipelineId);
                //TODO setup type for subpass
                /*
                if (exportSubPass.type == core::pb::Pipeline_Type_NONE)
                {
                    exportSubPass->set_type(pipeline->info.pipeline().type());
                }
                else
                {
                    if (exportSubPass->type() != pipeline->info.pipeline().type())
                    {
                        LogWarning(std::format("Subpass {} cannot have different pipeline types (Rasterize, Compute or Raytracing)", subPassIndex));
                        return false;
                    }
                }
                */
                auto pipelineIndexIt = resourceIndexMap.find(pipeline->resourceId);
                if (pipelineIndexIt == resourceIndexMap.end())
                {
                    const auto pipelineIndex = exportScene.pipelines.size();
                    renderer::GraphicsPipelineT exportPipeline{};
                    resourceIndexMap[pipeline->resourceId] = pipelineIndex;
                    const auto shaderExportFunc = [&resourceIndexMap, &exportScene, &shaderEditor](ResourceId shaderId)
                    {
                        if (shaderId == INVALID_RESOURCE_ID)
                        {
                            return -1;
                        }
                        const auto* shader = shaderEditor->GetShader(shaderId);
                        const auto shaderIt = resourceIndexMap.find(shaderId);
                        if (shaderIt == resourceIndexMap.end())
                        {
                            const auto shaderIndex = exportScene.shaders.size();
                            renderer::ShaderT exportShader{};
                            exportShader = shader->info;
                            exportShader.path = (shader->info.path);
                            resourceIndexMap[shader->resourceId] = shaderIndex;
                            exportScene.shaders.push_back(std::move(exportShader));
                            return (int)shaderIndex;
                        }
                        else
                        {
                            return shaderIt->second;
                        }
                    };
                    //switch (newPipeline->type())
                    //{
                    //case core::pb::Pipeline_Type_RASTERIZE:
                    {
                        int vertexShaderIndex = shaderExportFunc(pipeline->vertexShaderId);
                        if (vertexShaderIndex == -1)
                        {
                            LogWarning(std::format("Could not export scene, missing vertex shader in pipeline. Pipeline: {}", pipeline->path));
                            return false;
                        }
                        exportPipeline.vertex_shader_index = (vertexShaderIndex);

                        int fragmentShaderIndex = shaderExportFunc(pipeline->fragmentShaderId);
                        if (fragmentShaderIndex == -1)
                        {
                            LogWarning(std::format("Could not export scene, missing fragment shader in pipeline. Pipeline: {}", pipeline->path));
                            return false;
                        }
                        exportPipeline.fragment_shader_index = (fragmentShaderIndex);

                        exportMaterial.pipeline_index = (pipelineIndex);
                        exportedPipelineIndex = pipelineIndex;
                        //break;
                    }
                    //TODO implement compute and raytracing pipeline
                    /*
                    case core::pb::Pipeline_Type_COMPUTE:
                    {

                        int computeShaderIndex = shaderExportFunc(pipeline->computeShaderId);
                        if (computeShaderIndex == -1)
                        {
                            LogWarning(std::format("Could not export scene, missing compute shader in pipeline. Pipeline: {}", pipeline->path));
                            return false;
                        }
                        newPipeline->set_compute_shader_index(computeShaderIndex);
                        newMaterial->set_pipeline_index(pipelineIndex);
                        exportedPipelineIndex = pipelineIndex;

                        break;
                    }

                    case core::pb::Pipeline_Type_RAYTRACING:
                    {
                        int raytracingPipelineIndex = exportScene.raytracing_pipelines_size();
                        
                        auto* raytracingPipeline = exportScene.add_raytracing_pipelines();
                        newPipeline->set_raytracing_pipeline_index(raytracingPipelineIndex);
                        *raytracingPipeline = pipeline->raytracingInfo.pipeline();

                        int rayGenShaderIndex = shaderExportFunc(pipeline->rayGenShaderId);
                        if (rayGenShaderIndex == -1)
                        {
                            LogWarning(std::format("Could not export scene, missing raygen shader in pipeline. Pipeline: {}", pipeline->path));
                            return false;
                        }
                        raytracingPipeline->set_ray_gen_shader_index(rayGenShaderIndex);

                        int missHitShaderIndex = shaderExportFunc(pipeline->missHitShaderId);
                        if (missHitShaderIndex == -1)
                        {
                            LogWarning(std::format("Could not export scene, missing miss hit shader in pipeline. Pipeline: {}", pipeline->path));
                            return false;
                        }
                        raytracingPipeline->set_miss_hit_shader_index(missHitShaderIndex);

                        int closestHitShaderIndex = shaderExportFunc(pipeline->closestHitShaderId);
                        if (closestHitShaderIndex == -1)
                        {
                            LogWarning(std::format("Could not export scene, missing closest hit shader in pipeline. Pipeline: {}", pipeline->path));
                            return false;
                        }
                        raytracingPipeline->set_closest_hit_shader_index(closestHitShaderIndex);

                        raytracingPipeline->set_any_hit_shader_index(shaderExportFunc(pipeline->anyHitShaderId));
                        raytracingPipeline->set_intersection_hit_shader_index(shaderExportFunc(pipeline->intersectionHitShaderId));


                        break;
                    }
                    default: break;
                    }
                    */
                    exportScene.pipelines.push_back(exportPipeline);
                    
                }
                else
                {
                    exportMaterial.pipeline_index = (pipelineIndexIt->second);

                    exportedPipelineIndex = pipelineIndexIt->second;
                }
                if (exportDrawCommand)
                {
                    exportDrawCommand->material_index = (int)materialIndex;
                }
                //TODO when compute command are added, set material index
                /*
                if(exportComputeCommand)
                {
                    exportComputeCommand->set_material_index(materialIndex);
                }
                */
                exportScene.materials.push_back(std::move(exportMaterial));
            }
            else
            {
                if (exportDrawCommand)
                {
                    exportDrawCommand->material_index = (materialIndexIt->second);
                }
                //TODO when compute command are added, set material index
                /*
                if (exportComputeCommand)
                {
                    exportComputeCommand->set_material_index(materialIndexIt->second);
                }
                */
            }
            if (exportDrawCommand)
            {
                //link mesh index
                if (editorCommand->meshId == INVALID_RESOURCE_ID)
                {
                    LogWarning(std::format("Could not export scene, missing mesh in command. Command: {}", editorCommand->path));
                    return false;
                }
                auto* mesh = meshEditor->GetMesh(editorCommand->meshId);
                //add model if not done already

                int meshModel = -1;
                // TODO export model mesh
                /*
                if (!mesh->info.model_path.empty())
                {
                    for (int modelIndex = 0; modelIndex < exportScene.model_paths.size(); modelIndex++)
                    {
                        auto& sceneModelPath = exportScene.model_paths(modelIndex);
                        if (fs::equivalent(sceneModelPath, mesh->info.model_path()))
                        {
                            meshModel = modelIndex;
                            break;
                        }
                    }
                    if (meshModel == -1)
                    {
                        meshModel = exportScene.model_paths.size();
                        exportScene.add_model_paths(mesh->info.model_path());
                    }
                    else
                    {
                        mesh->info.mutable_mesh()->set_model_index(meshModel);
                    }
                }
                */
                auto meshIndexIt = resourceIndexMap.find(mesh->resourceId);
                if (meshIndexIt == resourceIndexMap.end())
                {
                    const auto meshIndex = exportScene.meshes.size();
                    renderer::MeshT newMesh{};
                    newMesh = *mesh->info.mesh;
                    newMesh.model_index = (meshModel);
                    resourceIndexMap[mesh->resourceId] = meshIndex;
                    exportDrawCommand->mesh_index = (meshIndex);
                    exportScene.meshes.push_back(std::move(newMesh));
                }
                else
                {
                    exportDrawCommand->mesh_index = (meshIndexIt->second);
                }
            }
            //link buffer
            if(editorCommand->bufferId == INVALID_RESOURCE_ID)
            {
                const auto* material = materialEditor->GetMaterial(editorCommand->materialId);
                const auto* pipeline = pipelineEditor->GetPipeline(material->pipelineId);
                const std::array resourceIds =
                {
                    pipeline->vertexShaderId,
                    pipeline->fragmentShaderId,
                    pipeline->computeShaderId,
                };
                for(auto resourceId: resourceIds)
                {
                    auto* shader = shaderEditor->GetShader(resourceId);
                    if(shader == nullptr)
                    {
                        continue;
                    }
                    if(!shader->info.storage_buffers.empty())
                    {
                        LogWarning(std::format("Could not export scene, missing buffer binding in command. Command: {}", editorCommand->path));
                        return false;
                    }
                }
            }
            else
            {
                const auto it = resourceIndexMap.find(editorCommand->bufferId);
                if(it == resourceIndexMap.end())
                {
                    auto* buffer = bufferEditor->GetBuffer(editorCommand->bufferId);
                    int bufferIndex = exportScene.buffers.size();
                    renderer::StorageBufferT exportBuffer{};
                    exportBuffer = buffer->info;
                    resourceIndexMap[editorCommand->bufferId] = bufferIndex;
                    exportScene.buffers.push_back(std::move(exportBuffer));
                }
                else
                {
                    //TODO add buffer index to draw command?
                    //FIXME binding buffer index in draw command? was it not material?
                    //exportDrawCommand->buffer_index = (it->second);
                }
            }
            exportSubPass.commands.push_back(std::move(*exportDrawCommand));
        }
    }

    for(size_t i = 0; i < currentScene.info.system_paths.size();i++)
    {
        const std::string_view systemPath = currentScene.info.system_paths[i];
        if(std::ranges::any_of(core::GetNativeScriptClassNames(), [&systemPath](auto nativeScript){return nativeScript == systemPath;}))
        {
            engine::SystemT system;
            system.module_ = core::nativeModuleName;
            system.class_ = systemPath;
        }
        else
        {
            const auto systemId = resourceManager.FindResourceByPath(systemPath);
            const auto* systemInfo = scriptEditor->GetScriptInfo(systemId);

            exportScene.systems.push_back(systemInfo->info);
        }
    }
    static constexpr std::string_view exportScenePath = "root.scene";
    //Write scene
    if (!core::WriteFlatbufferToFile<renderer::SceneT, renderer::Scene>(exportScene, exportScenePath))
    {
        LogWarning(std::format("Could not save scene for export at: {}", exportScenePath));
        return false;
    }
    //Create scene json
    json sceneJson;
    sceneJson["scene"] = exportScenePath.data();
    std::vector<std::string> shaderPaths;
    shaderPaths.reserve(exportScene.shaders.size());
    for(size_t i = 0; i < exportScene.shaders.size(); i++)
    {
        shaderPaths.push_back(fs::path(exportScene.shaders[i].path, std::filesystem::path::generic_format).string());
    }
    sceneJson["shaders"] = shaderPaths;
    std::vector<std::string> scriptPaths;
    scriptPaths.reserve(exportScene.systems.size());
    for (size_t i = 0; i < exportScene.systems.size(); i++)
    {
        if(exportScene.systems[i].path.empty())
            continue;
        scriptPaths.push_back(fs::path(exportScene.systems[i].path, std::filesystem::path::generic_format).string());
    }
    sceneJson["scripts"] = scriptPaths;
    std::vector<std::string> texturePaths;
    texturePaths.reserve(exportScene.textures.size());
    for(size_t i = 0; i < exportScene.textures.size(); i++)
    {
        texturePaths.push_back(exportScene.textures[i].path);
    }
    sceneJson["textures"] = texturePaths;
    sceneJson["cubemap_textures"] = cubemapTextures;
    std::vector<std::string> objPaths;
    //TODO add models
    /*
    objPaths.reserve(exportScene.model_paths.size());
    for(int i = 0; i < exportScene.model_paths_size(); i++)
    {
        objPaths.push_back(exportScene.model_paths(i));
    }
    */
    sceneJson["models"] = objPaths;

    std::vector<std::string> others;
    //TODO add models
    /*
    for(auto& objFile : objPaths)
    {
        const auto modelPath{
            std::format("{}/{}.model", GetFolder(objFile), GetFilename(objFile, false))};
        const auto modelId = resourceManager.FindResourceByPath(modelPath);
        auto* model = modelEditor->GetModel(modelId);
        for(int i = 0; i < model->info.mtl_paths_size(); i++)
        {
            others.push_back(model->info.mtl_paths(i));
        }
    }
    */
    sceneJson["others"] = others;

    //Call python function exporting the scene
    try
    {
        py::function exportSceneFunc = py::module_::import("scripts.generate_scene").attr("export_scene");
        exportSceneFunc(pkgSceneName, sceneJson.dump());
    }
    catch (py::error_already_set& e)
    {
        LogError(e.what());
        return false;
    }
    RemoveFile(exportScenePath);
    ExecutePlayer(pkgSceneName);
    return true;
}

void SceneEditor::ReloadId()
{
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    for(auto& sceneInfo: sceneInfos_)
    {
        if (sceneInfo.renderPassId == INVALID_RESOURCE_ID && !sceneInfo.info.renderpass_path.empty())
        {
            sceneInfo.renderPassId = resourceManager.FindResourceByPath(sceneInfo.info.renderpass_path);
        }
    }
}

void SceneEditor::Delete()
{
    if (currentIndex_ >= sceneInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.RemoveResource(sceneInfos_[currentIndex_].path, true);
}

std::span<const std::string_view> SceneEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = { ".scene" };
    return std::span{ extensions };
}

void SceneEditor::Clear()
{
    sceneInfos_.clear();
    currentIndex_ = -1;
}



SceneEditor* GetSceneEditor()
{
    const auto* editor = Editor::GetInstance();
    return static_cast<SceneEditor*>(editor->GetEditorSystem(EditorType::SCENE));
}
}
