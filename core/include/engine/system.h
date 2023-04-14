#pragma once

#include <string_view>
#include <glm/vec2.hpp>
#include <SDL2/SDL_keycode.h>

#include "utils/locator.h"
#include "utils/log.h"


namespace core
{
class System
{
    public:
    virtual ~System() = default;

    virtual void Begin() = 0;
    virtual void Update(float dt) = 0;
    virtual void End() = 0;
};

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

    std::string_view GetName() const { return name_; }
    void SetName(std::string_view newName) { name_ = newName; }
protected:
    std::string name_;
};

class ScriptLoaderInterface
{
public:
    virtual Script* LoadScript(
        std::string_view path, 
        std::string_view module, 
        std::string_view className) = 0;
    virtual void Begin() = 0;
    virtual void End() = 0;

};

class NullScriptLoader final : public ScriptLoaderInterface
{
public:
    Script* LoadScript(std::string_view path, std::string_view module, std::string_view className) override
    {
        LogError("Did not provide a Script Loader");
        std::terminate();
        return nullptr;
    }

    void Begin() override
    {
        LogError("Did not provide a Script Loader");
        std::terminate();
    }
    void End() override
    {
        LogError("Did not provide a Script Loader");
        std::terminate();
    }
};

class ScriptLoaderLocator : public Locator<ScriptLoaderInterface, NullScriptLoader>
{
    
};
} // namespace core
