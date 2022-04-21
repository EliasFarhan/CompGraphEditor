#include "scene_editor.h"
#include <imgui.h>

namespace gpr5300
{

void SceneEditor::DrawImGui()
{
    auto* engine = Engine::GetInstance();
    const auto windowSize = engine->GetWindowSize();


    ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x*0.2f, windowSize.y), ImGuiCond_FirstUseEver);
    DrawSceneContent();

    ImGui::SetNextWindowPos(ImVec2(windowSize.x*0.2f,0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x*0.6f, windowSize.y), ImGuiCond_FirstUseEver);
    ImGui::Begin("Center View", nullptr, ImGuiWindowFlags_NoTitleBar);
    DrawMenuBar();
    DrawCenterView();
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(windowSize.x*0.8f,0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x*0.2f, windowSize.y), ImGuiCond_FirstUseEver);
    DrawInspector();
}
void SceneEditor::Begin()
{

}
void SceneEditor::Update(float dt)
{

}
void SceneEditor::End()
{

}
void SceneEditor::DrawMenuBar()
{
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Window"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
void SceneEditor::DrawSceneContent()
{
    ImGui::Begin("Scene Content", nullptr, ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("Scene Content");

    if(ImGui::TreeNode("Shaders"))
    {
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Pipelines"))
    {
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Materials"))
    {
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Textures"))
    {
        ImGui::TreePop();
    }

    ImGui::End();
}
void SceneEditor::DrawCenterView()
{
    if(ImGui::BeginTabBar("Center View"))
    {
        if(ImGui::BeginTabItem("Shader"))
        {
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Pipeline"))
        {
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Material"))
        {
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Texture"))
        {
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}
void SceneEditor::DrawInspector()
{

}
}