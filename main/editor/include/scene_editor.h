#pragma once

#include "engine/system.h"
#include "engine/engine.h"
#include "engine/scene.h"

#include <imgui.h>
#include "imfilebrowser.h"

namespace gpr5300
{

class Editor : public System, public ImguiDrawInterface, public OnEventInterface
{
public:
    void DrawImGui() override;
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void OnEvent(SDL_Event &event) override;
private:
    void DrawMenuBar();
    void DrawEditorContent();
    void DrawSceneContent();
    void DrawCenterView();
    void DrawInspector();
    void DrawLogWindow();
    void UpdateFileDialog();
    void LoadFileIntoEditor(std::string_view path);

    pb::Scene scene_;
    ResourceManager resourceManager_;
    ImGui::FileBrowser fileDialog_;

};

}