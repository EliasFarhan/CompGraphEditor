#include "pipeline_editor.h"
#include "editor.h"
#include "shader_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"
#include <fmt/format.h>
#include <imgui_stdlib.h>
#include <fstream>

namespace gpr5300
{
    
void PipelineEditor::DrawInspector()
{
    if (currentIndex_ >= pipelineInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& currentPipelineInfo = pipelineInfos_[currentIndex_];

    //Pipeline type
    if(currentPipelineInfo.info.type() != core::pb::Pipeline_Type_RASTERIZE &&
        currentPipelineInfo.info.type() != core::pb::Pipeline_Type_COMPUTE)
    {
        currentPipelineInfo.info.set_type(core::pb::Pipeline_Type_RASTERIZE);
    }
    int index = currentPipelineInfo.info.type();
    const char* pipelineTypeText[] = {
        "Rasterizer",
        "Compute"
    };
    if(ImGui::Combo("Combo", &index, pipelineTypeText, IM_ARRAYSIZE(pipelineTypeText)))
    {
        currentPipelineInfo.info.set_type(static_cast<core::pb::Pipeline_Type>(index));
    }

    //Rasterizer pipeline
    if (currentPipelineInfo.info.type() == core::pb::Pipeline_Type_RASTERIZE)
    {
        if(currentPipelineInfo.vertexShaderId == INVALID_RESOURCE_ID || 
            currentPipelineInfo.fragmentShaderId == INVALID_RESOURCE_ID)
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Pipeline is not completed (require at Vertex and Fragment shader)");
        }
        const auto* shaderEditor = dynamic_cast<ShaderEditor*>(editor->GetEditorSystem(EditorType::SHADER));
        const auto& shaders = shaderEditor->GetShaders();
        const auto* vertexShader = shaderEditor->GetShader(currentPipelineInfo.vertexShaderId);
        if (ImGui::BeginCombo("Vertex Shader", vertexShader ? vertexShader->filename.data() : "No vertex shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != core::pb::Shader_Type_VERTEX)
                {
                    continue;
                }
                if (ImGui::Selectable(shader.filename.c_str(), shader.resourceId == currentPipelineInfo.vertexShaderId))
                {
                    currentPipelineInfo.vertexShaderId = shader.resourceId;
                    currentPipelineInfo.info.set_vertex_shader_path(shader.info.path());
                    ReloadPipeline(currentIndex_);
                }
            }
            ImGui::EndCombo();
        }

        const auto* fragmentShader = shaderEditor->GetShader(currentPipelineInfo.fragmentShaderId);
        if (ImGui::BeginCombo("Fragment Shader", fragmentShader ? fragmentShader->filename.data() : "No fragment shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != core::pb::Shader_Type_FRAGMENT)
                {
                    continue;
                }
                if (ImGui::Selectable(shader.filename.c_str(), shader.resourceId == currentPipelineInfo.fragmentShaderId))
                {
                    currentPipelineInfo.fragmentShaderId = shader.resourceId;
                    currentPipelineInfo.info.set_fragment_shader_path(shader.info.path());
                    ReloadPipeline(currentIndex_);
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();
        bool depthTesting = currentPipelineInfo.info.depth_test_enable();
        if(ImGui::Checkbox("Depth Testing", &depthTesting))
        {
            currentPipelineInfo.info.set_depth_test_enable(depthTesting);
        }
        if(depthTesting)
        {
            static constexpr std::array<std::string_view, 8> depthCompareOpNames =
            {
                "LESS",
                "LESS_OR_EQUAL",
                "EQUAL",
                "GREATER",
                "NOT_EQUAL",
                "GREATER_OR_EQUAL",
                "ALWAYS",
                "NEVER"
            };
            int index = currentPipelineInfo.info.depth_compare_op();
            if(ImGui::BeginCombo("Depth Compare Op", depthCompareOpNames[index].data()))
            {
                for (std::size_t i = 0; i < depthCompareOpNames.size(); i++)
                {
                    if (ImGui::Selectable(depthCompareOpNames[i].data(), i == index))
                    {
                        currentPipelineInfo.info.set_depth_compare_op(static_cast<core::pb::Pipeline_DepthCompareOp>(i));
                    }
                }
                ImGui::EndCombo();
            }
            bool depthMask = currentPipelineInfo.info.depth_mask();
            if(ImGui::Checkbox("Depth Mask", &depthMask))
            {
                currentPipelineInfo.info.set_depth_mask(depthMask);
            }
        }
        ImGui::Separator();
        bool stencilEnable = currentPipelineInfo.info.enable_stencil_test();
        if(ImGui::Checkbox("Enable Stencil Test", &stencilEnable))
        {
            currentPipelineInfo.info.set_enable_stencil_test(stencilEnable);
        }
        if(stencilEnable)
        {
            static constexpr std::array<std::string_view, 8> stencilFuncTxt = {
                "NEVER",
                "LESS",
                "LEQUAL",
                "GREATER",
                "GEQUAL",
                "EQUAL",
                "NOTEQUAL",
                "ALWAYS"
            };
            if(ImGui::BeginCombo("Stencil Func", stencilFuncTxt[currentPipelineInfo.info.stencil_func()].data()))
            {
                for(std::size_t i = 0; i < stencilFuncTxt.size(); ++i)
                {
                    if(ImGui::Selectable(stencilFuncTxt[i].data(), i == currentPipelineInfo.info.stencil_func()))
                    {
                        currentPipelineInfo.info.set_stencil_func(static_cast<core::pb::Pipeline_StencilFunc>(i));
                    }
                }
                ImGui::EndCombo();
            }
            int stencilRef = currentPipelineInfo.info.stencil_ref();
            if(ImGui::InputInt("Stencil Ref", &stencilRef))
            {
                currentPipelineInfo.info.set_stencil_ref(stencilRef);
            }
            unsigned stencilFuncMask = currentPipelineInfo.info.stencil_func_mask();
            if(ImGui::InputScalar("Stencil Func Mask", ImGuiDataType_U32, &stencilFuncMask, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal))
            {
                currentPipelineInfo.info.set_stencil_func_mask(stencilFuncMask);
            }
            const auto stencilFuncCommand = fmt::format("glStencilFunc(GL_{},{},0x{:X});", stencilFuncTxt[currentPipelineInfo.info.stencil_func()], stencilRef, stencilFuncMask);
            ImGui::Text("%s", stencilFuncCommand.data());
            //Stencil op
            static constexpr std::array<std::string_view, 8> stencilOpTxt =
            {
                "KEEP",
                "ZERO",
                "REPLACE",
                "INCR",
                "INCR_WRAP",
                "DECR",
                "DECR_WRAP",
                "INVERT"
            };

            if(ImGui::BeginCombo("Source Fail", stencilOpTxt[currentPipelineInfo.info.stencil_source_fail()].data()))
            {
                for(std::size_t i = 0; i < stencilOpTxt.size(); i++)
                {
                    if(ImGui::Selectable(stencilOpTxt[i].data(), i == currentPipelineInfo.info.stencil_source_fail()))
                    {
                        currentPipelineInfo.info.set_stencil_source_fail(static_cast<core::pb::Pipeline_StencilOp>(i));
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Depth Fail", stencilOpTxt[currentPipelineInfo.info.stencil_depth_fail()].data()))
            {
                for (std::size_t i = 0; i < stencilOpTxt.size(); i++)
                {
                    if (ImGui::Selectable(stencilOpTxt[i].data(), i == currentPipelineInfo.info.stencil_depth_fail()))
                    {
                        currentPipelineInfo.info.set_stencil_depth_fail(static_cast<core::pb::Pipeline_StencilOp>(i));
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Depth Pass", stencilOpTxt[currentPipelineInfo.info.stencil_depth_pass()].data()))
            {
                for (std::size_t i = 0; i < stencilOpTxt.size(); i++)
                {
                    if (ImGui::Selectable(stencilOpTxt[i].data(), i == currentPipelineInfo.info.stencil_depth_pass()))
                    {
                        currentPipelineInfo.info.set_stencil_depth_pass(static_cast<core::pb::Pipeline_StencilOp>(i));
                    }
                }
                ImGui::EndCombo();
            }
            const auto stencilOpCommand = fmt::format("glStencilOp(GL_{}, GL_{}, GL_{});",
                stencilOpTxt[currentPipelineInfo.info.stencil_source_fail()],
                stencilOpTxt[currentPipelineInfo.info.stencil_depth_fail()],
                stencilOpTxt[currentPipelineInfo.info.stencil_depth_pass()]);
            ImGui::Text("%s", stencilOpCommand.data());

            unsigned stencilMask = currentPipelineInfo.info.stencil_mask();
            if (ImGui::InputScalar("Stencil Mask", ImGuiDataType_U32, &stencilMask, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal))
            {
                currentPipelineInfo.info.set_stencil_mask(stencilMask);
            }
            const auto stencilMaskCommand = fmt::format("glStencilMask(0x{:X});", currentPipelineInfo.info.stencil_mask());
            ImGui::Text("%s", stencilMaskCommand.data());
        }

        ImGui::Separator();
        bool enableBlend = currentPipelineInfo.info.blend_enable();
        if(ImGui::Checkbox("Enable Blend", &enableBlend))
        {
            currentPipelineInfo.info.set_blend_enable(enableBlend);
        }
        if(enableBlend)
        {
            static constexpr std::array<std::string_view, 19> blendFuncTxt =
            {
                "ZERO",
                "ONE",
                "SRC_COLOR",
                "ONE_MINUS_SRC_COLOR",
                "DST_COLOR",
                "ONE_MINUS_DST_COLOR",
                "SRC_ALPHA",
                "ONE_MINUS_SRC_ALPHA",
                "DST_ALPHA",
                "ONE_MINUS_DST_ALPHA",
                "CONSTANT_COLOR",
                "ONE_MINUS_CONSTANT_COLOR",
                "CONSTANT_ALPHA",
                "ONE_MINUS_CONSTANT_ALPHA",
                "SRC_ALPHA_SATURATE",
                "SRC1_COLOR",
                "ONE_MINUS_SRC1_COLOR",
                "SRC1_ALPHA",
                "ONE_MINUS_SRC1_ALPHA"
            };
            if(ImGui::BeginCombo("Source Factor", blendFuncTxt[currentPipelineInfo.info.blending_source_factor()].data()))
            {
                for(std::size_t i = 0; i < blendFuncTxt.size(); ++i)
                {
                    if(ImGui::Selectable(blendFuncTxt[i].data(), i == currentPipelineInfo.info.blending_source_factor()))
                    {
                        currentPipelineInfo.info.set_blending_source_factor(static_cast<core::pb::Pipeline_BlendFunc>(i));
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Destination Factor", blendFuncTxt[currentPipelineInfo.info.blending_destination_factor()].data()))
            {
                for (std::size_t i = 0; i < blendFuncTxt.size(); ++i)
                {
                    if (ImGui::Selectable(blendFuncTxt[i].data(), i == currentPipelineInfo.info.blending_destination_factor()))
                    {
                        currentPipelineInfo.info.set_blending_destination_factor(static_cast<core::pb::Pipeline_BlendFunc>(i));
                    }
                }
                ImGui::EndCombo();
            }
            const auto blendFuncCommand = fmt::format("glBlendFunc(GL_{}, GL_{});", 
                blendFuncTxt[currentPipelineInfo.info.blending_source_factor()], 
                blendFuncTxt[currentPipelineInfo.info.blending_destination_factor()]);
            ImGui::Text("%s", blendFuncCommand.c_str());
        }
        ImGui::Separator();
        bool enableCulling = currentPipelineInfo.info.enable_culling();
        if(ImGui::Checkbox("Enable Culling", &enableCulling))
        {
            currentPipelineInfo.info.set_enable_culling(enableCulling);
        }
        if(enableCulling)
        {
            static constexpr std::array<std::string_view, 3> cullFaceTxt =
            {
                "BACK",
                "FRONT",
                "BACK_AND_FRONT"
            };
            if(ImGui::BeginCombo("Cull Face", cullFaceTxt[currentPipelineInfo.info.cull_face()].data()))
            {
                for(std::size_t i = 0; i < cullFaceTxt.size(); i++)
                {
                    if(ImGui::Selectable(cullFaceTxt[i].data(), i == currentPipelineInfo.info.cull_face()))
                    {
                        currentPipelineInfo.info.set_cull_face(static_cast<core::pb::Pipeline_CullFace>(i));
                    }
                }
                ImGui::EndCombo();
            }
            static constexpr std::array<std::string_view, 2> frontFaceTxt =
            {
                "COUNTER CLOCKWISE",
                "CLOCKWISE"
            };
            if(ImGui::BeginCombo("Front Face", frontFaceTxt[currentPipelineInfo.info.front_face()].data()))
            {
                for(std::size_t i = 0; i < frontFaceTxt.size(); i++)
                {
                    if(ImGui::Selectable(frontFaceTxt[i].data(), i == currentPipelineInfo.info.front_face()))
                    {
                        currentPipelineInfo.info.set_front_face(static_cast<core::pb::Pipeline_FrontFace>(i));
                    }
                }
                ImGui::EndCombo();
            }
        }
        ImGui::Separator();
        static constexpr std::array<std::string_view, 15> textureTypesTxt =
        {
                "NONE",
                "AMBIENT",
                "DIFFUSE",
                "SPECULAR",
                "SPECULAR_HIGHLIGHT",
                "BUMP",
                "DISPLACEMENT",
                "ALPHA",
                "REFLECTION",
                "ROUGHNESS",
                "METALLIC",
                "SHEEN",
                "EMISSIVE",
                "NORMAL",
                "AO"
        };

        if(ImGui::BeginTable("Samplers Table", 2))
        {
            for(int i = 0; i < currentPipelineInfo.info.samplers_size(); i++)
            {
                auto* sampler = currentPipelineInfo.info.mutable_samplers(i);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Name: %s", sampler->name().c_str());
                ImGui::TableSetColumnIndex(1);
                const auto comboId = fmt::format("Sampler Texture Type {}", i);
                ImGui::PushID(comboId.data());
                if(ImGui::BeginCombo("Texture Type", textureTypesTxt[static_cast<int>(sampler->type())].data()))
                {
                    for(std::size_t j = 0; j < textureTypesTxt.size(); j++)
                    {
                        if(ImGui::Selectable(textureTypesTxt[j].data(), j == sampler->type()))
                        {
                            sampler->set_type(static_cast<core::pb::TextureType>(j));
                            auto* materialEditor = editor->GetEditorSystem(EditorType::MATERIAL);
                            auto& resourceManager = editor->GetResourceManager();
                            const auto* pipelineResource = resourceManager.GetResource(currentPipelineInfo.resourceId);
                            if (pipelineResource != nullptr)
                            {
                                materialEditor->UpdateExistingResource(*pipelineResource);
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }


        ImGui::Separator();
        if (ImGui::BeginListBox("Uniforms"))
        {
            for (int i = 0; i < currentPipelineInfo.info.uniforms_size(); i++)
            {
                const auto& uniformInfo = currentPipelineInfo.info.uniforms(i);
                const auto text = fmt::format("Name: {} Type: {}", uniformInfo.name(), uniformInfo.type_name());
                ImGui::Selectable(text.c_str(), false);
            }
            
            ImGui::EndListBox();
        }

        if (ImGui::BeginListBox("In Attributes"))
        {
            for (int i = 0; i < currentPipelineInfo.info.in_vertex_attributes_size(); i++)
            {
                const auto& inAttributeInfo = currentPipelineInfo.info.in_vertex_attributes(i);
                const auto text = fmt::format("Name: {} Type: {}", inAttributeInfo.name(), inAttributeInfo.type_name());
                ImGui::Selectable(text.c_str(), false);
            }
            ImGui::EndListBox();
        }
    }
    else if(currentPipelineInfo.info.type() == core::pb::Pipeline_Type_COMPUTE)
    {
        
    }
}
bool PipelineEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = pipelineInfos_.size();
    for (std::size_t i = 0; i < pipelineInfos_.size(); i++)
    {
        const auto& shaderInfo = pipelineInfos_[i];
        if (ImGui::Selectable(shaderInfo.filename.data(),  currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}
std::string_view PipelineEditor::GetSubFolder()
{
    return "pipelines/";
}
EditorType PipelineEditor::GetEditorType()
{
    return EditorType::PIPELINE;
}
void PipelineEditor::AddResource(const Resource& resource)
{
    PipelineInfo pipelineInfo{};
    pipelineInfo.filename = GetFilename(resource.path);
    pipelineInfo.resourceId = resource.resourceId;
    pipelineInfo.path = resource.path;

    const auto& fileSystem = core::FilesystemLocator::get();

    if (!fileSystem.IsRegularFile(resource.path))
    {
        LogWarning(fmt::format("Could not find pipeline file: {}", resource.path));
        return;
    }
    std::ifstream fileIn (resource.path, std::ios::binary);
    if (!pipelineInfo.info.ParseFromIstream(&fileIn))
    {
        LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
        return;
    }
    pipelineInfo.info.set_name(GetFilename(resource.path, false));
    pipelineInfos_.push_back(pipelineInfo);
}
void PipelineEditor::RemoveResource(const Resource& resource)
{
    int i = 0; 
    for(auto& pipelineInfo : pipelineInfos_)
    {
        bool modified = false;
        if(pipelineInfo.vertexShaderId == resource.resourceId)
        {
            pipelineInfo.vertexShaderId = INVALID_RESOURCE_ID;
            pipelineInfo.info.clear_vertex_shader_path();
            modified = true;
        }
        if (pipelineInfo.fragmentShaderId == resource.resourceId)
        {
            pipelineInfo.fragmentShaderId = INVALID_RESOURCE_ID;
            pipelineInfo.info.clear_fragment_shader_path();
            modified = true;
        }
        if(modified)
        {
            ReloadPipeline(i);
        }
        i++;
    }

    const auto it = std::ranges::find_if(pipelineInfos_, [&resource](const auto& pipelineInfo)
        {
            return resource.resourceId == pipelineInfo.resourceId;
        });
    if(it != pipelineInfos_.end())
    {
        pipelineInfos_.erase(it);
        const auto* editor = Editor::GetInstance();
        auto* materialEditor = editor->GetEditorSystem(EditorType::MATERIAL);
        materialEditor->RemoveResource(resource);
    }
}
void PipelineEditor::UpdateExistingResource(const Resource& resource)
{
    //update pipeline if shader modifies it
    int i = 0;
    for (const auto& pipelineInfo : pipelineInfos_)
    {
        bool modified = false;
        if (pipelineInfo.vertexShaderId == resource.resourceId)
        {
            modified = true;
        }
        if (pipelineInfo.fragmentShaderId == resource.resourceId)
        {
            modified = true;
        }
        if (modified)
        {
            ReloadPipeline(i);
            const auto* editor = Editor::GetInstance();
            const auto& resourceManager = editor->GetResourceManager();
            auto* materialEditor = editor->GetEditorSystem(EditorType::MATERIAL);

            const auto* pipelineResource = resourceManager.GetResource(pipelineInfo.resourceId);
            if (pipelineResource != nullptr)
            {
                materialEditor->UpdateExistingResource(*pipelineResource);
            }
        }
        i++;
    }
}

void PipelineEditor::Save()
{
    for(auto& pipelineInfo : pipelineInfos_)
    {
        std::ofstream fileOut(pipelineInfo.path, std::ios::binary);
        if (!pipelineInfo.info.SerializeToOstream(&fileOut))
        {
            LogWarning(fmt::format("Could not save pipeline at: {}", pipelineInfo.path));
        }
        
    }
}

const PipelineInfo *PipelineEditor::GetPipeline(ResourceId resourceId) const {
    const auto it = std::ranges::find_if(pipelineInfos_, [resourceId](const auto& pipeline)
    {
        return resourceId == pipeline.resourceId;
    });
    if(it != pipelineInfos_.end())
    {
        return &*it;
    }
    return nullptr;
}
void PipelineEditor::ReloadId()
{
    
    for (std::size_t i = 0; i < pipelineInfos_.size();i++)
    {
        ReloadPipeline(i);
    }
}

void PipelineEditor::Delete()
{
    if (currentIndex_ >= pipelineInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.RemoveResource(pipelineInfos_[currentIndex_].path);
}

std::span<const std::string_view> PipelineEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = { ".pipe" };
    return std::span{ extensions };
}

void PipelineEditor::ReloadPipeline(int index)
{
    if(index >= pipelineInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    const auto& resourceManager = editor->GetResourceManager();
    const auto* shaderEditor = dynamic_cast<ShaderEditor*>(editor->GetEditorSystem(EditorType::SHADER));

    auto& pipelineInfo = pipelineInfos_[index];
    if (pipelineInfo.info.type() == core::pb::Pipeline_Type_RASTERIZE)
    {
        if (pipelineInfo.vertexShaderId == INVALID_RESOURCE_ID && !pipelineInfo.info.vertex_shader_path().empty())
        {
            pipelineInfo.vertexShaderId = resourceManager.FindResourceByPath(pipelineInfo.info.vertex_shader_path());
        }

        if (pipelineInfo.fragmentShaderId == INVALID_RESOURCE_ID && !pipelineInfo.info.fragment_shader_path().empty())
        {
            pipelineInfo.fragmentShaderId = resourceManager.FindResourceByPath(pipelineInfo.info.fragment_shader_path());
        }
    }
    std::vector<core::pb::Sampler> samplers;
    samplers.reserve(pipelineInfo.info.samplers_size());
    for(int i = 0; i < pipelineInfo.info.samplers_size(); i++)
    {
        samplers.push_back(pipelineInfo.info.samplers(i));
    }
    pipelineInfo.info.mutable_samplers()->Clear();
    pipelineInfo.info.mutable_uniforms()->Clear();
    pipelineInfo.info.mutable_in_vertex_attributes()->Clear();
    //load uniforms and input attributes to pipeline
    if(pipelineInfo.vertexShaderId != INVALID_RESOURCE_ID)
    {
        const auto* vertexShader = shaderEditor->GetShader(pipelineInfo.vertexShaderId);
        for(int i = 0; i < vertexShader->info.in_attributes_size(); i++)
        {
            *pipelineInfo.info.add_in_vertex_attributes() = vertexShader->info.in_attributes(i);
        }
        for(int i = 0; i < vertexShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.add_uniforms() = vertexShader->info.uniforms(i);
        }
    }
    if(pipelineInfo.fragmentShaderId != INVALID_RESOURCE_ID)
    {
        const auto* fragmentShader = shaderEditor->GetShader(pipelineInfo.fragmentShaderId);
        for (int i = 0; i < fragmentShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.add_uniforms() = fragmentShader->info.uniforms(i);
        }
    }
    for(int i = 0; i < pipelineInfo.info.uniforms_size(); i++)
    {
        const auto& uniform = pipelineInfo.info.uniforms(i);
        const auto uniformType = uniform.type();
        if(uniformType != core::pb::Attribute_Type_SAMPLER2D && uniformType != core::pb::Attribute_Type_SAMPLERCUBE)
        {
            continue;
        }
        const core::pb::Sampler* previousSampler = nullptr;
        for(auto& sampler : samplers)
        {
            if(sampler.name() == uniform.name())
            {
                previousSampler = &sampler;
                break;
            }
        }
        auto* newSampler = pipelineInfo.info.add_samplers();
        if(previousSampler != nullptr)
        {
            *newSampler = *previousSampler;
        }
        else
        {
            newSampler->set_name(uniform.name());
            newSampler->set_type(core::pb::NONE);
        }
    }
}
}
