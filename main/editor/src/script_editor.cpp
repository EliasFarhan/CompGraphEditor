#include "script_editor.h"

#include "utils/log.h"
#include "editor.h"
#include "scene_editor.h"

#include <imgui.h>
#include <fmt/format.h>

#include <nlohmann/json.hpp>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

// for convenience
using json = nlohmann::json;
namespace gpr5300
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
        LogDebug(fmt::format("Loading script: {} with content:\n{}", resource.path, result));
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
                LogDebug(fmt::format("Loading script: {} with content:\n{}", resource.path, result));
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

bool ScriptEditor::CheckExtensions(std::string_view extension)
{
    return extension == ".py";
}

void ScriptEditor::DrawMainView()
{
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

bool ScriptEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = scriptInfos_.size();
    for (std::size_t i = 0; i < scriptInfos_.size(); i++)
    {
        const auto& shaderInfo = scriptInfos_[i];
        if (ImGui::Selectable(shaderInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
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
} // namespace grp5300
