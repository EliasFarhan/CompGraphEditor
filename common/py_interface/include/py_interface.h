#pragma once
#include <pybind11/pybind11.h>

#include "engine/script.h"
#include "utils/log.h"

#include "renderer/draw_command.h"


namespace py = pybind11;

namespace core
{

class DrawCommand;

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

    void Dispatch(Command* command) override
    {
        try
        {
            PYBIND11_OVERRIDE_NAME(
                void, /* Return type */
                Script,      /* Parent class */
                "dispatch",
                Dispatch      /* Name of the function(s) */,
                command
            );
        }
        catch (py::error_already_set& e)
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

class PyManager final : public MinimalScriptLoader
{
public:
    PyManager();
    void Begin() override;
    void End() override;
    Script* LoadScript(std::string_view path, std::string_view module, std::string_view className) override;
private:
    std::vector<py::object> pySystems_;
    bool initialized = false;
};

} // namespace core
