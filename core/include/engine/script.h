#pragma once

#include <glm/vec2.hpp>
#include <SDL2/SDL_keycode.h>

#include "utils/locator.h"
#include "utils/log.h"
#include "engine/system.h"


#include <memory>
#include <string_view>
#include <unordered_map>
#include <span>

namespace core
{
class Command;

class ComputeCommand;
class DrawCommand;

class Script : public System
{
public:
    virtual void Draw(DrawCommand* sceneDrawCommand) {}
    virtual void OnKeyDown(SDL_Keycode keycode) {}
    virtual void OnKeyUp(SDL_Keycode keycode) {}
    virtual void OnMouseMotion(glm::vec2 mouseMotion) {}
    virtual void OnMouseButtonDown(int mouseButton) {}
    virtual void OnMouseButtonUp(int mouseButton) {}
    virtual void Dispatch(ComputeCommand* command){}
    virtual void Trace(Command* command){}
};

class ScriptLoaderInterface
{
public:
    virtual ~ScriptLoaderInterface() = default;
    virtual Script* LoadScript(
        std::string_view path, 
        std::string_view module, 
        std::string_view className) = 0;
    virtual void Begin() = 0;
    virtual void End() = 0;
    virtual void ImportScript(std::string_view module, 
        std::string_view className, 
        std::unique_ptr<Script> script) = 0;

};

class NullScriptLoader final : public ScriptLoaderInterface
{
public:
    Script* LoadScript(std::string_view path, std::string_view module, std::string_view className) override;

    void Begin() override;

    void End() override;

    void ImportScript(std::string_view module, std::string_view className, std::unique_ptr<Script> script) override;
};

class MinimalScriptLoader : public ScriptLoaderInterface
{
public:
    MinimalScriptLoader();
    Script* LoadScript(std::string_view path, std::string_view module, std::string_view className) override;
    void Begin() override;
    void End() override;
    void ImportScript(std::string_view module, std::string_view className, std::unique_ptr<Script> script) override;
private:
    std::unordered_map<std::string, std::unique_ptr<Script>> nativeScripts_;
};

class ScriptLoaderLocator : public Locator<ScriptLoaderInterface, NullScriptLoader>
{
    
};


constexpr std::string_view nativeModuleName = "cppmodule";

std::span<const std::string_view> GetNativeScriptClassNames();

void ImportNativeScript();

} // namespace core
