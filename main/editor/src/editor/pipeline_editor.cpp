#include "pipeline_editor.h"
#include "editor.h"
#include "shader_editor.h"
#include "scene_editor.h"
#include "engine/filesystem.h"
#include "utils/log.h"
#include <fmt/format.h>
#include <imgui_stdlib.h>
#include <fstream>

#include "imnodes.h"

namespace editor
{
    
void PipelineEditor::DrawInspector()
{
    if (currentIndex_ >= pipelineInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& currentPipelineInfo = pipelineInfos_[currentIndex_];

    const bool isVulkan = GetSceneEditor()->IsVulkanScene();
    //Pipeline type
    if(!isVulkan && currentPipelineInfo.info.pipeline().type() != core::pb::Pipeline_Type_RASTERIZE &&
        currentPipelineInfo.info.pipeline().type() != core::pb::Pipeline_Type_COMPUTE)
    {
        currentPipelineInfo.info.mutable_pipeline()->set_type(core::pb::Pipeline_Type_RASTERIZE);
    }
    int index = currentPipelineInfo.info.pipeline().type();
    const char* pipelineTypeText[] = {
        "Rasterizer",
        "Compute",
        "Raytracing"
    };
    if(ImGui::Combo("Combo", &index, pipelineTypeText, isVulkan ? 3 : 2))
    {
        currentPipelineInfo.info.mutable_pipeline()->set_type(static_cast<core::pb::Pipeline_Type>(index));
    }

    const auto* shaderEditor = dynamic_cast<ShaderEditor*>(editor->GetEditorSystem(EditorType::SHADER));
    const auto& shaders = shaderEditor->GetShaders();
    //Rasterizer pipeline
    if (currentPipelineInfo.info.pipeline().type() == core::pb::Pipeline_Type_RASTERIZE)
    {
        if(currentPipelineInfo.vertexShaderId == INVALID_RESOURCE_ID || 
            currentPipelineInfo.fragmentShaderId == INVALID_RESOURCE_ID)
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Pipeline is not completed (require at Vertex and Fragment shader)");
        }
        const auto* vertexShader = shaderEditor->GetShader(currentPipelineInfo.vertexShaderId);
        if (ImGui::BeginCombo("Vertex Shader", vertexShader ? vertexShader->filename.data() : "No vertex shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != core::pb::VERTEX)
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
                if (shader.info.type() != core::pb::FRAGMENT)
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
        const auto* geometryShader = shaderEditor->GetShader(currentPipelineInfo.geometryShaderId);
        if (ImGui::BeginCombo("Geometry Shader", geometryShader ? geometryShader->filename.data() : "No Geometry shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != core::pb::GEOMETRY)
                {
                    continue;
                }
                if (ImGui::Selectable(shader.filename.c_str(), shader.resourceId == currentPipelineInfo.geometryShaderId))
                {
                    currentPipelineInfo.geometryShaderId = shader.resourceId;
                    currentPipelineInfo.info.set_geometry_shader_path(shader.info.path());
                    ReloadPipeline(currentIndex_);
                }
            }
            ImGui::EndCombo();
        }
        const auto* tessControlShader = shaderEditor->GetShader(currentPipelineInfo.tessControlShaderId);
        if (ImGui::BeginCombo("Tesselation Control Shader", tessControlShader ? tessControlShader->filename.data() : "No Tesselation Control shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != core::pb::TESSELATION_CONTROL)
                {
                    continue;
                }
                if (ImGui::Selectable(shader.filename.c_str(), shader.resourceId == currentPipelineInfo.tessControlShaderId))
                {
                    currentPipelineInfo.tessControlShaderId = shader.resourceId;
                    currentPipelineInfo.info.set_tess_control_shader_path(shader.info.path());
                    ReloadPipeline(currentIndex_);
                }
            }
            ImGui::EndCombo();
        }
        const auto* tessEvalShader = shaderEditor->GetShader(currentPipelineInfo.tessEvalShaderId);
        if (ImGui::BeginCombo("Tesselation Evaluation Shader", tessEvalShader ? tessEvalShader->filename.data() : "No Tesselation Evaluation shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != core::pb::TESSELATION_EVAL)
                {
                    continue;
                }
                if (ImGui::Selectable(shader.filename.c_str(), shader.resourceId == currentPipelineInfo.tessEvalShaderId))
                {
                    currentPipelineInfo.tessEvalShaderId = shader.resourceId;
                    currentPipelineInfo.info.set_tess_eval_shader_path(shader.info.path());
                    ReloadPipeline(currentIndex_);
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Separator();
        bool depthTesting = currentPipelineInfo.info.pipeline().depth_test_enable();

        if(ImGui::Checkbox("Depth Testing", &depthTesting))
        {
            currentPipelineInfo.info.mutable_pipeline()->set_depth_test_enable(depthTesting);
            currentPipelineInfo.info.mutable_pipeline()->set_depth_mask(true);
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
            int index = currentPipelineInfo.info.pipeline().depth_compare_op();
            if(ImGui::BeginCombo("Depth Compare Op", depthCompareOpNames[index].data()))
            {
                for (std::size_t i = 0; i < depthCompareOpNames.size(); i++)
                {
                    if (ImGui::Selectable(depthCompareOpNames[i].data(), i == index))
                    {
                        currentPipelineInfo.info.mutable_pipeline()->set_depth_compare_op(static_cast<core::pb::Pipeline_DepthCompareOp>(i));
                    }
                }
                ImGui::EndCombo();
            }
            bool depthMask = currentPipelineInfo.info.pipeline().depth_mask();
            if(ImGui::Checkbox("Depth Mask", &depthMask))
            {
                currentPipelineInfo.info.mutable_pipeline()->set_depth_mask(depthMask);
            }
        }
        ImGui::Separator();
        bool stencilEnable = currentPipelineInfo.info.pipeline().enable_stencil_test();
        if(ImGui::Checkbox("Enable Stencil Test", &stencilEnable))
        {
            currentPipelineInfo.info.mutable_pipeline()->set_enable_stencil_test(stencilEnable);
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
            if(ImGui::BeginCombo("Stencil Func", stencilFuncTxt[currentPipelineInfo.info.pipeline().stencil_func()].data()))
            {
                for(std::size_t i = 0; i < stencilFuncTxt.size(); ++i)
                {
                    if(ImGui::Selectable(stencilFuncTxt[i].data(), i == currentPipelineInfo.info.pipeline().stencil_func()))
                    {
                        currentPipelineInfo.info.mutable_pipeline()->set_stencil_func(static_cast<core::pb::Pipeline_StencilFunc>(i));
                    }
                }
                ImGui::EndCombo();
            }
            int stencilRef = currentPipelineInfo.info.pipeline().stencil_ref();
            if(ImGui::InputInt("Stencil Ref", &stencilRef))
            {
                currentPipelineInfo.info.mutable_pipeline()->set_stencil_ref(stencilRef);
            }
            unsigned stencilFuncMask = currentPipelineInfo.info.pipeline().stencil_func_mask();
            if(ImGui::InputScalar("Stencil Func Mask", ImGuiDataType_U32, &stencilFuncMask, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal))
            {
                currentPipelineInfo.info.mutable_pipeline()->set_stencil_func_mask(stencilFuncMask);
            }
            const auto stencilFuncCommand = fmt::format("glStencilFunc(GL_{},{},0x{:X});", stencilFuncTxt[currentPipelineInfo.info.pipeline().stencil_func()], stencilRef, stencilFuncMask);
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

            if(ImGui::BeginCombo("Source Fail", stencilOpTxt[currentPipelineInfo.info.pipeline().stencil_source_fail()].data()))
            {
                for(std::size_t i = 0; i < stencilOpTxt.size(); i++)
                {
                    if(ImGui::Selectable(stencilOpTxt[i].data(), i == currentPipelineInfo.info.pipeline().stencil_source_fail()))
                    {
                        currentPipelineInfo.info.mutable_pipeline()->set_stencil_source_fail(static_cast<core::pb::Pipeline_StencilOp>(i));
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Depth Fail", stencilOpTxt[currentPipelineInfo.info.pipeline().stencil_depth_fail()].data()))
            {
                for (std::size_t i = 0; i < stencilOpTxt.size(); i++)
                {
                    if (ImGui::Selectable(stencilOpTxt[i].data(), i == currentPipelineInfo.info.pipeline().stencil_depth_fail()))
                    {
                        currentPipelineInfo.info.mutable_pipeline()->set_stencil_depth_fail(static_cast<core::pb::Pipeline_StencilOp>(i));
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Depth Pass", stencilOpTxt[currentPipelineInfo.info.pipeline().stencil_depth_pass()].data()))
            {
                for (std::size_t i = 0; i < stencilOpTxt.size(); i++)
                {
                    if (ImGui::Selectable(stencilOpTxt[i].data(), i == currentPipelineInfo.info.pipeline().stencil_depth_pass()))
                    {
                        currentPipelineInfo.info.mutable_pipeline()->set_stencil_depth_pass(static_cast<core::pb::Pipeline_StencilOp>(i));
                    }
                }
                ImGui::EndCombo();
            }
            const auto stencilOpCommand = fmt::format("glStencilOp(GL_{}, GL_{}, GL_{});",
                stencilOpTxt[currentPipelineInfo.info.pipeline().stencil_source_fail()],
                stencilOpTxt[currentPipelineInfo.info.pipeline().stencil_depth_fail()],
                stencilOpTxt[currentPipelineInfo.info.pipeline().stencil_depth_pass()]);
            ImGui::Text("%s", stencilOpCommand.data());

            unsigned stencilMask = currentPipelineInfo.info.pipeline().stencil_mask();
            if (ImGui::InputScalar("Stencil Mask", ImGuiDataType_U32, &stencilMask, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal))
            {
                currentPipelineInfo.info.mutable_pipeline()->set_stencil_mask(stencilMask);
            }
            const auto stencilMaskCommand = fmt::format("glStencilMask(0x{:X});", currentPipelineInfo.info.pipeline().stencil_mask());
            ImGui::Text("%s", stencilMaskCommand.data());
        }

        ImGui::Separator();
        bool enableBlend = currentPipelineInfo.info.pipeline().blend_enable();
        if(ImGui::Checkbox("Enable Blend", &enableBlend))
        {
            currentPipelineInfo.info.mutable_pipeline()->set_blend_enable(enableBlend);
            currentPipelineInfo.info.mutable_pipeline()->set_blending_source_factor(core::pb::Pipeline_BlendFunc_SRC_ALPHA);
            currentPipelineInfo.info.mutable_pipeline()->set_blending_destination_factor(core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC_ALPHA);
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
            if(ImGui::BeginCombo("Source Factor", blendFuncTxt[currentPipelineInfo.info.pipeline().blending_source_factor()].data()))
            {
                for(std::size_t i = 0; i < blendFuncTxt.size(); ++i)
                {
                    if(ImGui::Selectable(blendFuncTxt[i].data(), i == currentPipelineInfo.info.pipeline().blending_source_factor()))
                    {
                        currentPipelineInfo.info.mutable_pipeline()->set_blending_source_factor(static_cast<core::pb::Pipeline_BlendFunc>(i));
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Destination Factor", blendFuncTxt[currentPipelineInfo.info.pipeline().blending_destination_factor()].data()))
            {
                for (std::size_t i = 0; i < blendFuncTxt.size(); ++i)
                {
                    if (ImGui::Selectable(blendFuncTxt[i].data(), i == currentPipelineInfo.info.pipeline().blending_destination_factor()))
                    {
                        currentPipelineInfo.info.mutable_pipeline()->set_blending_destination_factor(static_cast<core::pb::Pipeline_BlendFunc>(i));
                    }
                }
                ImGui::EndCombo();
            }
            const auto blendFuncCommand = fmt::format("glBlendFunc(GL_{}, GL_{});", 
                blendFuncTxt[currentPipelineInfo.info.pipeline().blending_source_factor()],
                blendFuncTxt[currentPipelineInfo.info.pipeline().blending_destination_factor()]);
            ImGui::Text("%s", blendFuncCommand.c_str());
        }
        ImGui::Separator();
        bool enableCulling = currentPipelineInfo.info.pipeline().enable_culling();
        if(ImGui::Checkbox("Enable Culling", &enableCulling))
        {
            currentPipelineInfo.info.mutable_pipeline()->set_enable_culling(enableCulling);
        }
        if(enableCulling)
        {
            static constexpr std::array<std::string_view, 3> cullFaceTxt =
            {
                "BACK",
                "FRONT",
                "BACK_AND_FRONT"
            };
            if(ImGui::BeginCombo("Cull Face", cullFaceTxt[currentPipelineInfo.info.pipeline().cull_face()].data()))
            {
                for(std::size_t i = 0; i < cullFaceTxt.size(); i++)
                {
                    if(ImGui::Selectable(cullFaceTxt[i].data(), i == currentPipelineInfo.info.pipeline().cull_face()))
                    {
                        currentPipelineInfo.info.mutable_pipeline()->set_cull_face(static_cast<core::pb::Pipeline_CullFace>(i));
                    }
                }
                ImGui::EndCombo();
            }
            static constexpr std::array<std::string_view, 2> frontFaceTxt =
            {
                "COUNTER CLOCKWISE",
                "CLOCKWISE"
            };
            if(ImGui::BeginCombo("Front Face", frontFaceTxt[currentPipelineInfo.info.pipeline().front_face()].data()))
            {
                for(std::size_t i = 0; i < frontFaceTxt.size(); i++)
                {
                    if(ImGui::Selectable(frontFaceTxt[i].data(), i == currentPipelineInfo.info.pipeline().front_face()))
                    {
                        currentPipelineInfo.info.mutable_pipeline()->set_front_face(static_cast<core::pb::Pipeline_FrontFace>(i));
                    }
                }
                ImGui::EndCombo();
            }
        }
        ImGui::Separator();


        if(ImGui::BeginTable("Samplers Table", 2))
        {
            for(int i = 0; i < currentPipelineInfo.info.pipeline().samplers_size(); i++)
            {
                auto* sampler = currentPipelineInfo.info.mutable_pipeline()->mutable_samplers(i);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Name: %s", sampler->name().c_str());
                ImGui::TableSetColumnIndex(1);
                const auto comboId = fmt::format("Sampler Texture Type {}", i);
                ImGui::PushID(comboId.data());
                auto textureTypeTxt = aiTextureTypeToString(static_cast<aiTextureType>(sampler->type()));
                if(ImGui::BeginCombo("Texture Type", textureTypeTxt))
                {
                    for(std::size_t j = 0; j < core::pb::TextureType::LENGTH; j++)
                    {
                        if(ImGui::Selectable(aiTextureTypeToString(static_cast<aiTextureType>(j)), j == sampler->type()))
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
            for (int i = 0; i < currentPipelineInfo.info.pipeline().uniforms_size(); i++)
            {
                const auto& uniformInfo = currentPipelineInfo.info.mutable_pipeline()->uniforms(i);
                const auto text = fmt::format("Name: {} Type: {}", uniformInfo.name(), uniformInfo.type_name());
                ImGui::Selectable(text.c_str(), false);
            }
            
            ImGui::EndListBox();
        }

        if (ImGui::BeginListBox("In Attributes"))
        {
            for (int i = 0; i < currentPipelineInfo.info.pipeline().in_vertex_attributes_size(); i++)
            {
                const auto& inAttributeInfo = currentPipelineInfo.info.mutable_pipeline()->in_vertex_attributes(i);
                const auto text = fmt::format("Name: {} Type: {}", inAttributeInfo.name(), inAttributeInfo.type_name());
                ImGui::Selectable(text.c_str(), false);
            }
            ImGui::EndListBox();
        }
    }
    else if(currentPipelineInfo.info.pipeline().type() == core::pb::Pipeline_Type_COMPUTE)
    {
        const auto* computeShader = shaderEditor->GetShader(currentPipelineInfo.computeShaderId);
        if (ImGui::BeginCombo("Compute Shader", computeShader ? computeShader->filename.data() : "No Compute shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != core::pb::COMPUTE)
                {
                    continue;
                }
                if (ImGui::Selectable(shader.filename.c_str(), shader.resourceId == currentPipelineInfo.computeShaderId))
                {
                    currentPipelineInfo.computeShaderId = shader.resourceId;
                    currentPipelineInfo.info.set_compute_shader_path(shader.info.path());
                    ReloadPipeline(currentIndex_);
                }
            }
            ImGui::EndCombo();
        }
    }
    else if(currentPipelineInfo.info.pipeline().type() == core::pb::Pipeline_Type_RAYTRACING)
    {
        const auto* rayGenShader = shaderEditor->GetShader(currentPipelineInfo.rayGenShaderId);
        if (ImGui::BeginCombo("Ray Gen Shader", rayGenShader ? rayGenShader->filename.data() : "No Ray Gen shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != core::pb::RAY_GEN)
                {
                    continue;
                }
                if (ImGui::Selectable(shader.filename.c_str(), shader.resourceId == currentPipelineInfo.rayGenShaderId))
                {
                    currentPipelineInfo.rayGenShaderId = shader.resourceId;
                    currentPipelineInfo.raytracingInfo.set_ray_gen_shader_path(shader.info.path());
                    ReloadPipeline(currentIndex_);
                }
            }
            ImGui::EndCombo();
        }
        const auto* missHitShader = shaderEditor->GetShader(currentPipelineInfo.missHitShaderId);
        if (ImGui::BeginCombo("Miss Hit Shader", missHitShader ? missHitShader->filename.data() : "No Miss Hit shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != core::pb::RAY_MISS)
                {
                    continue;
                }
                if (ImGui::Selectable(shader.filename.c_str(), shader.resourceId == currentPipelineInfo.missHitShaderId))
                {
                    currentPipelineInfo.missHitShaderId = shader.resourceId;
                    currentPipelineInfo.raytracingInfo.set_miss_hit_shader_path(shader.info.path());
                    ReloadPipeline(currentIndex_);
                }
            }
            ImGui::EndCombo();
        }
        const auto* closestHitShader = shaderEditor->GetShader(currentPipelineInfo.closestHitShaderId);
        if (ImGui::BeginCombo("Closest Hit Shader", closestHitShader ? closestHitShader->filename.data() : "No Closest Hit shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != core::pb::RAY_CLOSEST_HIT)
                {
                    continue;
                }
                if (ImGui::Selectable(shader.filename.c_str(), shader.resourceId == currentPipelineInfo.closestHitShaderId))
                {
                    currentPipelineInfo.closestHitShaderId = shader.resourceId;
                    currentPipelineInfo.raytracingInfo.set_closest_hit_shader_path(shader.info.path());
                    ReloadPipeline(currentIndex_);
                }
            }
            ImGui::EndCombo();
        }
        const auto* anyHitShader = shaderEditor->GetShader(currentPipelineInfo.anyHitShaderId);
        if (ImGui::BeginCombo("Any Hit Shader", closestHitShader ? closestHitShader->filename.data() : "No Any Hit shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != core::pb::RAY_ANY_HIT)
                {
                    continue;
                }
                if (ImGui::Selectable(shader.filename.c_str(), shader.resourceId == currentPipelineInfo.anyHitShaderId))
                {
                    currentPipelineInfo.anyHitShaderId = shader.resourceId;
                    currentPipelineInfo.raytracingInfo.set_any_hit_shader_path(shader.info.path());
                    ReloadPipeline(currentIndex_);
                }
            }
            ImGui::EndCombo();
        }
        const auto* intersectionShader = shaderEditor->GetShader(currentPipelineInfo.intersectionHitShaderId);
        if (ImGui::BeginCombo("Intersection Hit Shader", intersectionShader ? intersectionShader->filename.data() : "No Intersection Hit shader"))
        {
            for (auto& shader : shaders)
            {
                if (shader.info.type() != core::pb::RAY_INTERSECTION)
                {
                    continue;
                }
                if (ImGui::Selectable(shader.filename.c_str(), shader.resourceId == currentPipelineInfo.intersectionHitShaderId))
                {
                    currentPipelineInfo.intersectionHitShaderId = shader.resourceId;
                    currentPipelineInfo.raytracingInfo.set_intersection_hit_shader_path(shader.info.path());
                    ReloadPipeline(currentIndex_);
                }
            }
            ImGui::EndCombo();
        }
        int maxRecursiveDepth = static_cast<int>(currentPipelineInfo.raytracingInfo.pipeline().max_recursion_depth());
        if(ImGui::InputInt("Max Recursive Depth", &maxRecursiveDepth))
        {
            currentPipelineInfo.raytracingInfo.mutable_pipeline()->set_max_recursion_depth(static_cast<std::uint32_t>(maxRecursiveDepth));
        }
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
    std::ifstream fileIn (resource.path.c_str(), std::ios::binary);
    if (!pipelineInfo.info.ParseFromIstream(&fileIn))
    {
        LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
        return;
    }
    
    pipelineInfo.info.mutable_pipeline()->set_name(GetFilename(resource.path, false));
    

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
        if(pipelineInfo.geometryShaderId == resource.resourceId)
        {
            pipelineInfo.geometryShaderId = INVALID_RESOURCE_ID;
            pipelineInfo.info.clear_geometry_shader_path();
            modified = true;
        }
        if(pipelineInfo.tessControlShaderId == resource.resourceId)
        {
            pipelineInfo.tessControlShaderId = INVALID_RESOURCE_ID;
            pipelineInfo.info.clear_tess_control_shader_path();
            modified = true;
        }
        if(pipelineInfo.tessEvalShaderId == resource.resourceId)
        {
            pipelineInfo.tessEvalShaderId = resource.resourceId;
            pipelineInfo.info.clear_tess_eval_shader_path();
            modified = true;
        }
        if(pipelineInfo.computeShaderId == resource.resourceId)
        {
            pipelineInfo.computeShaderId = INVALID_RESOURCE_ID;
            pipelineInfo.info.clear_compute_shader_path();
            modified = true;
        }
        if (pipelineInfo.rayGenShaderId == resource.resourceId)
        {
            pipelineInfo.rayGenShaderId = INVALID_RESOURCE_ID;
            pipelineInfo.raytracingInfo.clear_ray_gen_shader_path();
            modified = true;
        }
        if (pipelineInfo.missHitShaderId == resource.resourceId)
        {
            pipelineInfo.missHitShaderId = INVALID_RESOURCE_ID;
            pipelineInfo.raytracingInfo.clear_miss_hit_shader_path();
            modified = true;
        }
        if (pipelineInfo.anyHitShaderId == resource.resourceId)
        {
            pipelineInfo.anyHitShaderId = INVALID_RESOURCE_ID;
            pipelineInfo.raytracingInfo.clear_any_hit_shader_path();
            modified = true;
        }
        if (pipelineInfo.closestHitShaderId == resource.resourceId)
        {
            pipelineInfo.closestHitShaderId = INVALID_RESOURCE_ID;
            pipelineInfo.raytracingInfo.clear_closest_hit_shader_path();
            modified = true;
        }
        if (pipelineInfo.intersectionHitShaderId == resource.resourceId)
        {
            pipelineInfo.intersectionHitShaderId = INVALID_RESOURCE_ID;
            pipelineInfo.raytracingInfo.clear_intersection_hit_shader_path();
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
        if(pipelineInfo.computeShaderId == resource.resourceId)
        {
            modified = true;
        }
        if(pipelineInfo.geometryShaderId == resource.resourceId)
        {
            modified = true;
        }
        if(pipelineInfo.tessControlShaderId == resource.resourceId)
        {
            modified = true;
        }
        if(pipelineInfo.tessEvalShaderId == resource.resourceId)
        {
            modified = true;
        }
        if(pipelineInfo.rayGenShaderId == resource.resourceId)
        {
            modified = true;
        }
        if(pipelineInfo.closestHitShaderId == resource.resourceId)
        {
            modified = true;
        }
        if(pipelineInfo.missHitShaderId == resource.resourceId)
        {
            modified = true;
        }
        if(pipelineInfo.intersectionHitShaderId == resource.resourceId)
        {
            modified = true;
        }
        if(pipelineInfo.anyHitShaderId == resource.resourceId)
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
        std::ofstream fileOut(pipelineInfo.path.c_str(), std::ios::binary);
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
    resourceManager.RemoveResource(pipelineInfos_[currentIndex_].path, true);
}

std::span<const std::string_view> PipelineEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = { ".pipe" };
    return std::span{ extensions };
}

void PipelineEditor::Clear()
{
    pipelineInfos_.clear();
    currentIndex_ = -1;
}

void PipelineEditor::DrawCenterView()
{
    if(currentIndex_ >= pipelineInfos_.size())
        return;
    auto& currentPipeline = pipelineInfos_[currentIndex_];
    const auto& resourceManager = GetResourceManager();
    const auto* shaderEditor = dynamic_cast<ShaderEditor*>(Editor::GetInstance()->GetEditorSystem(EditorType::SHADER));
    
    ImNodes::BeginNodeEditor();

    std::vector<std::pair<int, int>> links;
    switch(currentPipeline.info.pipeline().type())
    {
    case core::pb::Pipeline_Type_RASTERIZE:
    {

        constexpr int vertexInputBaseIndex = 100;
        constexpr int vertexOutputBaseIndex = 300;
        constexpr int uniformsBaseIndex = 200;

        ImNodes::BeginNode(-1);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("Vertex Inputs");
        ImNodes::EndNodeTitleBar();
        for (int i = 0; i < currentPipeline.info.pipeline().in_vertex_attributes_size(); i++)
        {

            ImNodes::BeginOutputAttribute(vertexInputBaseIndex + i + 1);
            const auto& vertexInput = currentPipeline.info.pipeline().in_vertex_attributes(i);
            ImGui::Text("%s %s", vertexInput.type_name().c_str(), vertexInput.name().c_str());
            ImNodes::EndOutputAttribute();
            links.emplace_back(vertexInputBaseIndex, vertexInputBaseIndex + i + 1);
        }
        ImNodes::EndNode();


        ImNodes::SetNodeGridSpacePos(-1, {50,50});

        //Vertex Shader Node
        ImNodes::BeginNode(0);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("Vertex Shader");
        ImNodes::EndNodeTitleBar();
        ImNodes::BeginInputAttribute(vertexInputBaseIndex);
        ImGui::TextUnformatted("Vertex Inputs");
        ImNodes::EndInputAttribute();
        ImNodes::BeginInputAttribute(uniformsBaseIndex);
        ImGui::TextUnformatted("Uniforms");
        ImNodes::EndInputAttribute();

        if(!currentPipeline.info.vertex_shader_path().empty())
        {
            ImGui::TextUnformatted("Outputs");
            const auto* vertexShaderInfo = shaderEditor->GetShader(
                    resourceManager.FindResourceByPath(core::Path(currentPipeline.info.vertex_shader_path())));
            for (int i = 0; i < vertexShaderInfo->info.out_attributes_size(); i++)
            {
                const auto& vertexOutput = vertexShaderInfo->info.out_attributes(i);
                ImNodes::BeginOutputAttribute(vertexOutputBaseIndex + i + 1);
                ImGui::Text("%s %s", vertexOutput.type_name().c_str(),
                    vertexOutput.name().c_str());
                ImNodes::EndOutputAttribute();
                links.emplace_back(vertexOutputBaseIndex, vertexOutputBaseIndex + i + 1);
            }
        }
        ImNodes::EndNode();

        ImNodes::SetNodeGridSpacePos(0, { 250.0f,150 });

        ImNodes::BeginNode(1);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("Fragment Shader");
        ImNodes::EndNodeTitleBar();
        ImNodes::BeginInputAttribute(vertexOutputBaseIndex);
        ImGui::TextUnformatted("Inputs");
        ImNodes::EndInputAttribute();
        ImNodes::BeginInputAttribute(uniformsBaseIndex+1);
        ImGui::TextUnformatted("Uniforms");
        ImNodes::EndInputAttribute();
        ImNodes::EndNode();

        ImNodes::SetNodeGridSpacePos(1, { 450.0f,150 });
        

        ImNodes::BeginNode(-2);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("Uniforms");
        ImNodes::EndNodeTitleBar();
        for (int i = 0; i < currentPipeline.info.pipeline().uniforms_size(); i++)
        {
            const int uniformIndex = uniformsBaseIndex + i + 5;
            ImNodes::BeginOutputAttribute(uniformIndex);
            const auto& uniform = currentPipeline.info.pipeline().uniforms(i);
            ImGui::Text("%s %s", uniform.type_name().c_str(), uniform.name().c_str());
            ImNodes::EndOutputAttribute();
            int shaderIndex = uniformsBaseIndex;
            switch(currentPipeline.info.pipeline().uniforms(i).stage())
            {
            case core::pb::VERTEX: break;
            case core::pb::FRAGMENT: shaderIndex += 1; break;
            case core::pb::GEOMETRY: shaderIndex += 2; break;
            case core::pb::TESSELATION_CONTROL: shaderIndex += 3; break;
            case core::pb::TESSELATION_EVAL: shaderIndex += 4; break;
            default: break;
            }
            links.emplace_back(shaderIndex, uniformIndex);
        }

        ImNodes::EndNode();

        ImNodes::SetNodeGridSpacePos(-2, { 50.0f,250.0f });

        //TODO add geometry shader node if shader exists
        //TODO add tesselation control shader node if shader exists
        //TODO add tesselation eval shader node if shader exists
        

        for(std::size_t i = 0; i < links.size(); i++)
        {
            ImNodes::Link(i, links[i].first, links[i].second);
        }
        break;
    }
    case core::pb::Pipeline_Type_COMPUTE: break;
    case core::pb::Pipeline_Type_RAYTRACING: break;
    default: ;
    }
    

    ImNodes::EndNodeEditor();
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
    if (pipelineInfo.info.pipeline().type() == core::pb::Pipeline_Type_RASTERIZE)
    {
        if (pipelineInfo.vertexShaderId == INVALID_RESOURCE_ID && !pipelineInfo.info.vertex_shader_path().empty())
        {
            pipelineInfo.vertexShaderId = resourceManager.FindResourceByPath(core::Path(pipelineInfo.info.vertex_shader_path()));
        }

        if (pipelineInfo.fragmentShaderId == INVALID_RESOURCE_ID && !pipelineInfo.info.fragment_shader_path().empty())
        {
            pipelineInfo.fragmentShaderId = resourceManager.FindResourceByPath(core::Path(pipelineInfo.info.fragment_shader_path()));
        }

        if(pipelineInfo.computeShaderId == INVALID_RESOURCE_ID && !pipelineInfo.info.compute_shader_path().empty())
        {
            pipelineInfo.computeShaderId = resourceManager.FindResourceByPath(core::Path(pipelineInfo.info.compute_shader_path()));
        }

        if(pipelineInfo.geometryShaderId == INVALID_RESOURCE_ID && !pipelineInfo.info.geometry_shader_path().empty())
        {
            pipelineInfo.geometryShaderId = resourceManager.FindResourceByPath(core::Path(pipelineInfo.info.geometry_shader_path()));
        }

        if(pipelineInfo.tessControlShaderId == INVALID_RESOURCE_ID && !pipelineInfo.info.tess_control_shader_path().empty())
        {
            pipelineInfo.tessControlShaderId = resourceManager.FindResourceByPath(core::Path(pipelineInfo.info.tess_control_shader_path()));
        }

        if(pipelineInfo.tessEvalShaderId == INVALID_RESOURCE_ID && !pipelineInfo.info.tess_eval_shader_path().empty())
        {
            pipelineInfo.tessEvalShaderId = resourceManager.FindResourceByPath(core::Path(pipelineInfo.info.tess_eval_shader_path()));
        }
    }
    else if(pipelineInfo.info.pipeline().type() == core::pb::Pipeline_Type_COMPUTE)
    {
        if (pipelineInfo.computeShaderId == INVALID_RESOURCE_ID && !pipelineInfo.info.compute_shader_path().empty())
        {
            pipelineInfo.computeShaderId = resourceManager.FindResourceByPath(core::Path(pipelineInfo.info.compute_shader_path()));
        }
    }
    else if(pipelineInfo.info.pipeline().type() == core::pb::Pipeline_Type_RAYTRACING)
    {
        if (pipelineInfo.rayGenShaderId == INVALID_RESOURCE_ID && !pipelineInfo.raytracingInfo.ray_gen_shader_path().empty())
        {
            pipelineInfo.rayGenShaderId = resourceManager.FindResourceByPath(core::Path(pipelineInfo.raytracingInfo.ray_gen_shader_path()));
        }
        if (pipelineInfo.missHitShaderId == INVALID_RESOURCE_ID && !pipelineInfo.raytracingInfo.miss_hit_shader_path().empty())
        {
            pipelineInfo.missHitShaderId = resourceManager.FindResourceByPath(core::Path(pipelineInfo.raytracingInfo.miss_hit_shader_path()));
        }
        if (pipelineInfo.closestHitShaderId == INVALID_RESOURCE_ID && !pipelineInfo.raytracingInfo.closest_hit_shader_path().empty())
        {
            pipelineInfo.closestHitShaderId = resourceManager.FindResourceByPath(core::Path(pipelineInfo.raytracingInfo.closest_hit_shader_path()));
        }
        if (pipelineInfo.anyHitShaderId == INVALID_RESOURCE_ID && !pipelineInfo.raytracingInfo.any_hit_shader_path().empty())
        {
            pipelineInfo.anyHitShaderId = resourceManager.FindResourceByPath(core::Path(pipelineInfo.raytracingInfo.any_hit_shader_path()));
        }
        if (pipelineInfo.intersectionHitShaderId == INVALID_RESOURCE_ID && !pipelineInfo.raytracingInfo.intersection_hit_shader_path().empty())
        {
            pipelineInfo.intersectionHitShaderId = resourceManager.FindResourceByPath(core::Path(pipelineInfo.raytracingInfo.intersection_hit_shader_path()));
        }
    }
    std::vector<core::pb::Sampler> samplers;
    samplers.reserve(pipelineInfo.info.pipeline().samplers_size());
    for(int i = 0; i < pipelineInfo.info.pipeline().samplers_size(); i++)
    {
        samplers.push_back(pipelineInfo.info.pipeline().samplers(i));
    }
    pipelineInfo.info.mutable_pipeline()->mutable_samplers()->Clear();
    pipelineInfo.info.mutable_pipeline()->mutable_uniforms()->Clear();
    pipelineInfo.info.mutable_pipeline()->mutable_in_vertex_attributes()->Clear();
    //load uniforms and input attributes to pipeline
    if(pipelineInfo.vertexShaderId != INVALID_RESOURCE_ID)
    {
        const auto* vertexShader = shaderEditor->GetShader(pipelineInfo.vertexShaderId);
        for(int i = 0; i < vertexShader->info.in_attributes_size(); i++)
        {
            *pipelineInfo.info.mutable_pipeline()->add_in_vertex_attributes() = vertexShader->info.in_attributes(i);
        }
        for(int i = 0; i < vertexShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.mutable_pipeline()->add_uniforms() = vertexShader->info.uniforms(i);
        }
    }
    if(pipelineInfo.fragmentShaderId != INVALID_RESOURCE_ID)
    {
        const auto* fragmentShader = shaderEditor->GetShader(pipelineInfo.fragmentShaderId);
        for (int i = 0; i < fragmentShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.mutable_pipeline()->add_uniforms() = fragmentShader->info.uniforms(i);
        }
    }
    if(pipelineInfo.geometryShaderId != INVALID_RESOURCE_ID)
    {
        const auto* geometryShader = shaderEditor->GetShader(pipelineInfo.geometryShaderId);
        for (int i = 0; i < geometryShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.mutable_pipeline()->add_uniforms() = geometryShader->info.uniforms(i);
        }
    }
    if(pipelineInfo.computeShaderId != INVALID_RESOURCE_ID)
    {
        const auto* computeShader = shaderEditor->GetShader(pipelineInfo.computeShaderId);
        for (int i = 0; i < computeShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.mutable_pipeline()->add_uniforms() = computeShader->info.uniforms(i);
        }
    }
    if(pipelineInfo.tessControlShaderId != INVALID_RESOURCE_ID)
    {
        const auto* tessControlShader = shaderEditor->GetShader(pipelineInfo.tessControlShaderId);
        for (int i = 0; i < tessControlShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.mutable_pipeline()->add_uniforms() = tessControlShader->info.uniforms(i);
        }
    }
    if(pipelineInfo.tessEvalShaderId != INVALID_RESOURCE_ID)
    {
        const auto* tessEvalShader = shaderEditor->GetShader(pipelineInfo.tessEvalShaderId);
        for (int i = 0; i < tessEvalShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.mutable_pipeline()->add_uniforms() = tessEvalShader->info.uniforms(i);
        }
    }
    if (pipelineInfo.rayGenShaderId != INVALID_RESOURCE_ID)
    {
        const auto* rayGenShader = shaderEditor->GetShader(pipelineInfo.rayGenShaderId);
        for (int i = 0; i < rayGenShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.mutable_pipeline()->add_uniforms() = rayGenShader->info.uniforms(i);
        }
    }
    if (pipelineInfo.missHitShaderId != INVALID_RESOURCE_ID)
    {
        const auto* missHitShader = shaderEditor->GetShader(pipelineInfo.missHitShaderId);
        for (int i = 0; i < missHitShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.mutable_pipeline()->add_uniforms() = missHitShader->info.uniforms(i);
        }
    }
    if (pipelineInfo.closestHitShaderId != INVALID_RESOURCE_ID)
    {
        const auto* closestHitShader = shaderEditor->GetShader(pipelineInfo.closestHitShaderId);
        for (int i = 0; i < closestHitShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.mutable_pipeline()->add_uniforms() = closestHitShader->info.uniforms(i);
        }
    }
    if (pipelineInfo.anyHitShaderId != INVALID_RESOURCE_ID)
    {
        const auto* anyHitShader = shaderEditor->GetShader(pipelineInfo.anyHitShaderId);
        for (int i = 0; i < anyHitShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.mutable_pipeline()->add_uniforms() = anyHitShader->info.uniforms(i);
        }
    }
    if (pipelineInfo.intersectionHitShaderId != INVALID_RESOURCE_ID)
    {
        const auto* intersectionHitShader = shaderEditor->GetShader(pipelineInfo.intersectionHitShaderId);
        for (int i = 0; i < intersectionHitShader->info.uniforms_size(); i++)
        {
            *pipelineInfo.info.mutable_pipeline()->add_uniforms() = intersectionHitShader->info.uniforms(i);
        }
    }
    for(int i = 0; i < pipelineInfo.info.pipeline().uniforms_size(); i++)
    {
        const auto& uniform = pipelineInfo.info.pipeline().uniforms(i);
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
        auto* newSampler = pipelineInfo.info.mutable_pipeline()->add_samplers();
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
