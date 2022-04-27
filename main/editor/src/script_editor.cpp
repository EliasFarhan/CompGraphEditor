#include "script_editor.h"

#include "utils/log.h"

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
    scriptInfo.path = resource.path;

    const auto analyzeScriptFunc = py::module_::import("scripts.script_parser").attr("analyze_script");
    try
    {
        std::string result = static_cast<py::str>(analyzeScriptFunc(resource.path));
        LogDebug(fmt::format("Loading script: {} with content:\n{}", resource.path, result));

    }
    catch(py::error_already_set& e)
    {
        LogWarning(fmt::format("Could not analyze script: {}", resource.path));
    }
}

void ScriptEditor::RemoveResource(const Resource& resource)
{
}

void ScriptEditor::UpdateExistingResource(const Resource& resource)
{
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
}

bool ScriptEditor::DrawContentList(bool unfocus)
{
    return false;
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
} // namespace grp5300
