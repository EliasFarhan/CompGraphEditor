#pragma once


#include "engine/system.h"
#include "utils/log.h"


#include <SDL_keycode.h>
#include <glm/vec2.hpp>

#include <pybind11/pybind11.h>


namespace py = pybind11;

namespace core
{
class DrawCommand;

class Script : public System
{
public:
    virtual void Draw(DrawCommand* sceneDrawCommand) {}
    virtual void OnKeyDown(SDL_Keycode keycode) {}
    virtual void OnKeyUp(SDL_Keycode keycode){}
    virtual void OnMouseMotion(glm::vec2 mouseMotion){}
    virtual void OnMouseButtonDown(int mouseButton){}
    virtual void OnMouseButtonUp(int mouseButton){}
};

class PySystem : public Script
{
public:
    void Begin() override
    {
        try
        {
            PYBIND11_OVERRIDE_PURE_NAME(
                    void, /* Return type */
                    Script,      /* Parent class */
                    "begin",
                    Begin      /* Argument(s) */
            );
        }
        catch (py::error_already_set& e)
        {
            LogError(e.what());
        }
    }

    void Update(float dt) override
    {
        try
        {
            PYBIND11_OVERRIDE_PURE_NAME(
                    void, /* Return type */
                    Script,      /* Parent class */
                    "update",
                    Update,
                    dt /* Argument(s) */
            );
        }
        catch (py::error_already_set& e)
        {
            LogError(e.what());
        }
    }

    void End() override
    {
        try
        {
            PYBIND11_OVERRIDE_PURE_NAME(
                    void, /* Return type */
                    Script,      /* Parent class */
                    "end",
                    End      /* Argument(s) */
            );
        }
        catch (py::error_already_set& e)
        {
            LogError(e.what());
        }
    }
    void Draw(DrawCommand* drawCommand) override
    {
        try
        {
            PYBIND11_OVERRIDE_NAME(
                void, /* Return type */
                Script,      /* Parent class */
                "draw",
                Draw      /* Name of the function(s) */,
                drawCommand
            );
        }
        catch(py::error_already_set& e)
        {
            LogError(e.what());
        }
    }

    void OnKeyDown(SDL_Keycode keycode) override
    {
        try
        {
            PYBIND11_OVERRIDE_NAME(
                void, /* Return type */
                Script,      /* Parent class */
                "on_key_down",
                OnKeyDown      /* Name of the function(s) */,
                keycode
            );
        }
        catch (py::error_already_set& e)
        {
            LogError(e.what());
        }
    }
    void OnKeyUp(SDL_Keycode keycode) override
    {
        try
        {
            PYBIND11_OVERRIDE_NAME(
                void, /* Return type */
                Script,      /* Parent class */
                "on_key_up",
                OnKeyUp      /* Name of the function(s) */,
                keycode
            );
        }
        catch (py::error_already_set& e)
        {
            LogError(e.what());
        }
    }

    void OnMouseMotion(glm::vec2 mouseMotion) override
    {
        try
        {
            PYBIND11_OVERRIDE_NAME(
                void, /* Return type */
                Script,      /* Parent class */
                "on_mouse_motion",
                OnMouseMotion      /* Name of the function(s) */,
                mouseMotion
            );
        }
        catch (py::error_already_set& e)
        {
            LogError(e.what());
        }
    }
};

class PyManager : public System
{
public:
    void Begin() override;

    void Update(float dt) override;

    void End() override;

    Script* LoadScript(std::string_view path, std::string_view module, std::string_view className);
private:
    std::vector<py::object> pySystems_;
};
}