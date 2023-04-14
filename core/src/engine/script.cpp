#include "engine/script.h"

namespace core
{
Script* NullScriptLoader::LoadScript(std::string_view path, std::string_view module, std::string_view className)
{
    LogError("Did not provide a Script Loader");
    std::terminate();
    return nullptr;
}

void NullScriptLoader::Begin()
{
    LogError("Did not provide a Script Loader");
    std::terminate();
}

void NullScriptLoader::End()
{
    LogError("Did not provide a Script Loader");
    std::terminate();
}

void NullScriptLoader::ImportScript(std::string_view module, std::string_view className, std::unique_ptr<Script> script)
{
    LogError("Did not provide a Script Loader");
    std::terminate();
}

MinimalScriptLoader::MinimalScriptLoader()
{
    ScriptLoaderLocator::provide(this);
}

Script* MinimalScriptLoader::LoadScript(std::string_view path, std::string_view module, std::string_view className)
{
    const auto it = nativeScripts_.find(fmt::format("{}/{}", module, className));
    if(it != nativeScripts_.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void MinimalScriptLoader::Begin()
{
}

void MinimalScriptLoader::End()
{
}

void MinimalScriptLoader::ImportScript(std::string_view module, std::string_view className,
    std::unique_ptr<Script> script)
{
    nativeScripts_[fmt::format("{}/{}", module, className)] = std::move(script);
}


constexpr std::array<std::string_view, 1> nativeScriptClassNames =
{
    {
        "CameraSystem"
    }
};


std::span<const std::string_view> GetNativeScriptClassNames()
{
    return nativeScriptClassNames;
}

void ImportNativeScript()
{
    auto& scriptLoader = ScriptLoaderLocator::get();
    scriptLoader.ImportScript(nativeModuleName, nativeScriptClassNames[0], std::make_unique<CameraSystem>());
}
} // namespace core
