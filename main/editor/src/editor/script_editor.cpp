#include "script_editor.h"

#include "utils/log.h"
#include "editor.h"
#include "scene_editor.h"
#include "engine/filesystem.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <fmt/format.h>

#include <nlohmann/json.hpp>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

// for convenience
using json = nlohmann::json;
namespace editor
{
void ScriptEditor::AddResource(const Resource& resource)
{
    ScriptInfo scriptInfo{};
    scriptInfo.resourceId = resource.resourceId;
    scriptInfo.filename = GetFilename(resource.path);
    auto module = GetFolder(resource.path);
    std::ranges::replace(module, '/', '.');
    std::ranges::replace(module, '\\', '.');
    module = fmt::format("{}.{}", module, GetFilename(resource.path, false));
    scriptInfo.info.set_module(module);
    scriptInfo.info.set_path(resource.path);

    const auto analyzeScriptFunc = py::module_::import("scripts.script_parser").attr("analyze_script");
    try
    {
        std::string result = static_cast<py::str>(analyzeScriptFunc(resource.path));
        json scriptJson = json::parse(result);
        for(auto& className : scriptJson["classes"])
        {
            scriptInfo.classesInScript.push_back(className.get<std::string>());
        }
        if(!scriptInfo.classesInScript.empty())
        {
            scriptInfo.info.set_class_(scriptInfo.classesInScript[0]);
        }
    }
    catch(py::error_already_set& e)
    {
        LogWarning(fmt::format("Could not analyze script: {}\n{}", resource.path, e.what()));
    }
    scriptInfos_.push_back(scriptInfo);
}

void ScriptEditor::RemoveResource(const Resource& resource)
{
    const auto it = std::ranges::find_if(scriptInfos_, [&resource](const auto& script)
        {
            return resource.resourceId == script.resourceId;
        });
    if (it != scriptInfos_.end())
    {
        scriptInfos_.erase(it);
        const auto* editor = Editor::GetInstance();
        auto* sceneEditor = dynamic_cast<SceneEditor*>(editor->GetEditorSystem(EditorType::SCENE));

        sceneEditor->RemoveResource(resource);
    }
}

void ScriptEditor::UpdateExistingResource(const Resource& resource)
{
    for(auto& scriptInfo : scriptInfos_)
    {
        if(scriptInfo.resourceId == resource.resourceId)
        {
            const auto analyzeScriptFunc = py::module_::import("scripts.script_parser").attr("analyze_script");
            try
            {
                std::string result = static_cast<py::str>(analyzeScriptFunc(resource.path));
                json scriptJson = json::parse(result);
                for (auto& className : scriptJson["classes"])
                {
                    scriptInfo.classesInScript.push_back(className.get<std::string>());
                }
                if (!scriptInfo.classesInScript.empty())
                {
                    scriptInfo.info.set_class_(scriptInfo.classesInScript[0]);
                }
            }
            catch (py::error_already_set& e)
            {
                LogWarning(fmt::format("Could not analyze script: {}\n{}", resource.path, e.what()));
            }
        }
    }
}


void ScriptEditor::DrawInspector()
{
    if (currentIndex_ >= scriptInfos_.size())
    {
        return;
    }

    const auto& currentScriptInfo = scriptInfos_[currentIndex_];

    ImGui::Text("Path: %s", currentScriptInfo.info.path().c_str());
    ImGui::Text("Module %s", currentScriptInfo.info.module().c_str());
    if(currentScriptInfo.info.class_().empty())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.f, 0.f, 1.0f), "Missing class name in script");
    }
    else
    {
        ImGui::Text("Class Name: %s", currentScriptInfo.info.class_().c_str());
    }


}

void ScriptEditor::DrawCenterView()
{
    if(currentIndex_ >= scriptInfos_.size())
        return;
    ImGui::InputTextMultiline("Shader Content",
        &scriptText_,
        ImGui::GetContentRegionAvail());

}

bool ScriptEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
    {
        currentIndex_ = scriptInfos_.size();
        scriptText_.clear();
    }
    for (std::size_t i = 0; i < scriptInfos_.size(); i++)
    {
        const auto& scriptInfo = scriptInfos_[i];
        if (ImGui::Selectable(scriptInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            auto& filesystem = core::FilesystemLocator::get();
            const auto scriptContent = filesystem.LoadFile(scriptInfo.info.path());
            scriptText_ = reinterpret_cast<const char*>(scriptContent.data);
            wasFocused = true;
        }
    }
    return wasFocused;
}

std::string_view ScriptEditor::GetSubFolder()
{
    return "scripts/";
}

EditorType ScriptEditor::GetEditorType()
{
    return EditorType::SCRIPT;
}

void ScriptEditor::Save()
{
    if (currentIndex_ >= scriptInfos_.size())
    {
        return;
    }
    const auto& filesystem = core::FilesystemLocator::get();
    filesystem.WriteString(scriptInfos_[currentIndex_].info.path(), scriptText_);
    auto& resourceManager = Editor::GetInstance()->GetResourceManager();
    auto* resource = resourceManager.GetResource(scriptInfos_[currentIndex_].resourceId);
    resourceManager.UpdateExistingResource(*resource);
    
}

void ScriptEditor::ReloadId()
{
}

const ScriptInfo* ScriptEditor::GetScriptInfo(ResourceId resourceId) const
{
    const auto it = std::ranges::find_if(scriptInfos_, [resourceId](const auto& scriptInfo)
        {
            return resourceId == scriptInfo.resourceId;
        });
    if(it != scriptInfos_.end())
    {
        return &*it;
    }
    return nullptr;
}

void ScriptEditor::Delete()
{
    if (currentIndex_ >= scriptInfos_.size())
    {
        return;
    }
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    resourceManager.RemoveResource(scriptInfos_[currentIndex_].info.path(), true);
}

std::span<const std::string_view> ScriptEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 1> extensions = { ".py" };
    return std::span{ extensions };
}

void ScriptEditor::Clear()
{
    scriptInfos_.clear();
    scriptText_.clear();
    currentIndex_ = -1;
}
} // namespace grp5300
