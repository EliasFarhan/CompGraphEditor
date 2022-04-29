#pragma once

#include "engine/system.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace gpr5300
{

class Script : public System
{
public:
    virtual void Draw(int subpassIndex) {};
};

class PySystem : public Script
{
public:
    void Begin() override
    {
        PYBIND11_OVERRIDE_PURE_NAME(
                void, /* Return type */
                Script,      /* Parent class */
                "begin",
                Begin      /* Argument(s) */
        );
    }

    void Update(float dt) override
    {
        PYBIND11_OVERRIDE_PURE_NAME(
                void, /* Return type */
                Script,      /* Parent class */
                "update",
                Update,
                dt /* Argument(s) */
        );
    }

    void End() override
    {
        PYBIND11_OVERRIDE_PURE_NAME(
                void, /* Return type */
                Script,      /* Parent class */
                "end",
                End      /* Argument(s) */
        );
    }
    void Draw(int subpassIndex) override
    {
        PYBIND11_OVERRIDE_NAME(
            void, /* Return type */
            Script,      /* Parent class */
            "draw",
            Draw      /* Argument(s) */,
            subpassIndex
        );
    }
};

class PyManager : public System
{
public:
    void Begin() override;

    void Update(float dt) override;

    void End() override;

    Script* LoadScript(std::string_view path, std::string_view className);
private:
    std::vector<py::object> pySystems_;
};
}