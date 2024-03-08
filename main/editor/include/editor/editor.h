#pragma once

#include "engine/system.h"
#include "engine/engine.h"
#include "engine/scene.h"

#include <imgui.h>
#include "imfilebrowser.h"
#include "resource.h"
#include "resource_manager.h"
#include "editor_system.h"
#include "proto/renderer.pb.h"

#include <memory>

namespace editor
{

class Editor : public core::System, public core::OnGuiInterface,
    public core::OnEventInterface, public ResourceChangeInterface
{
public:
    Editor();
    void OnGui() override;
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void OnEvent(SDL_Event &event) override;
    void AddResource(const Resource &resource) override;
    void RemoveResource(const Resource &resource) override;
    void UpdateExistingResource(const Resource &resource) override;
    EditorSystem* GetEditorSystem(EditorType type) const;
    ResourceManager& GetResourceManager() { return resourceManager_; }
    const ResourceManager& GetResourceManager() const { return resourceManager_; }
    static Editor* GetInstance() { return instance_; }
    void CreateNewFile(const core::Path &path, EditorType type);
private:
    void OpenMenuCreateNewFile(EditorType editorType, std::string_view extension = "");
    void SaveProject() const;
    void DrawMenuBar();
    bool UpdateCreateNewFile();
    void DrawEditorContent();
    void DrawCenterView();
    void DrawInspector();
    static void DrawLogWindow();
    void UpdateFileDialog();
    void LoadFileIntoEditor(const core::Path &path);
    void RecursiveSceneFileReload();
    void OpenFileBrowserDialog(std::span<const std::string_view> extensions);



    EditorSystem* FindEditorSystem(const core::Path &path) const;
    
    ResourceManager resourceManager_;
    ImGui::FileBrowser fileDialog_;
    std::vector<std::unique_ptr<EditorSystem>> editorSystems_;
    EditorType currentFocusedSystem_ = EditorType::LENGTH;
    EditorType currentCreateFileSystem_ = EditorType::LENGTH;
    std::string newCreateFilename_;
    std::string newCreateExtension_;
    int currentExtensionCreateFileIndex_ = 0;

    inline static Editor* instance_ = nullptr;
};

ResourceManager& GetResourceManager();

void HelpMarker(const char* desc);
}
