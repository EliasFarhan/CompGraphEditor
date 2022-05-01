#pragma once

#include "engine/system.h"
#include "utils/log.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace gpr5300
{

class Script : public System
{
public:
    virtual void Draw(int subpassIndex) {}
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
    void Draw(int subpassIndex) override
    {
        try
        {
            PYBIND11_OVERRIDE_NAME(
                void, /* Return type */
                Script,      /* Parent class */
                "draw",
                Draw      /* Name of the function(s) */,
                subpassIndex
            );
        }
        catch(py::error_already_set& e)
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