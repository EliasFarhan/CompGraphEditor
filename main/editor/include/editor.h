#pragma once

#include "engine/system.h"
#include "engine/engine.h"
#include "engine/scene.h"

#include <imgui.h>
#include "imfilebrowser.h"
#include "resource.h"
#include "resource_manager.h"
#include "editor_system.h"

#include <memory>

namespace gpr5300
{

class Editor : public System, public ImguiDrawInterface, public OnEventInterface, public ResourceChangeInterface
{
public:
    void DrawImGui() override;
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void OnEvent(SDL_Event &event) override;
    void AddResource(const Resource &resource) override;
    void RemoveResource(const Resource &resource) override;
    void UpdateResource(const Resource &resource) override;
private:
    void DrawMenuBar();
    void DrawEditorContent();
    void DrawSceneContent();
    void DrawCenterView();
    void DrawInspector();
    void DrawLogWindow();
    void UpdateFileDialog();
    void LoadFileIntoEditor(std::string_view path);

    enum class FileBrowserMode
    {
        OPEN_FILE,
        CREATE_NEW_SHADER,
        NONE
    };

    EditorSystem* FindEditorSystem(std::string_view path);

    pb::Scene scene_;
    ResourceManager resourceManager_;
    ImGui::FileBrowser fileDialog_;
    EditorType currentFocusedSystem_ = EditorType::LENGTH;
    std::vector<std::unique_ptr<EditorSystem>> editorSystems_;

    FileBrowserMode fileBrowserMode_ = FileBrowserMode::NONE;
};

}