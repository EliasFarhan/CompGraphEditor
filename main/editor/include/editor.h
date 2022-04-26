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
    Editor();
    void DrawImGui() override;
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void OnEvent(SDL_Event &event) override;
    void AddResource(const Resource &resource) override;
    void RemoveResource(const Resource &resource) override;
    void UpdateResource(const Resource &resource) override;
    EditorSystem* GetEditorSystem(EditorType type) const;
    const ResourceManager& GetResourceManager() { return resourceManager_; }
    static Editor* GetInstance() { return instance_; }
private:
    void SaveProject();
    void DrawMenuBar();
    void DrawEditorContent();
    void DrawCenterView();
    void DrawInspector();
    void DrawLogWindow();
    void UpdateFileDialog();
    void LoadFileIntoEditor(std::string_view path);
    enum class FileBrowserMode
    {
        OPEN_FILE,
        CREATE_NEW_SHADER,
        CREATE_NEW_PIPELINE,
        CREATE_NEW_MATERIAL,
        CREATE_NEW_MESH,
        NONE
    };
    void OpenFileBrowserDialog(FileBrowserMode mode);



    EditorSystem* FindEditorSystem(std::string_view path) const;

    pb::Scene scene_;
    ResourceManager resourceManager_;
    ImGui::FileBrowser fileDialog_;
    EditorType currentFocusedSystem_ = EditorType::LENGTH;
    std::vector<std::unique_ptr<EditorSystem>> editorSystems_;

    FileBrowserMode fileBrowserMode_ = FileBrowserMode::NONE;

    inline static Editor* instance_ = nullptr;
};

}