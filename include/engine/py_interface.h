#pragma once

#include "engine/system.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace gpr5300
{

class PySystem : public System
{
public:
    void Begin() override
    {
        PYBIND11_OVERRIDE_PURE_NAME(
                void, /* Return type */
                System,      /* Parent class */
                "begin",
                Begin      /* Argument(s) */
        );
    }

    void Update(float dt) override
    {
        PYBIND11_OVERRIDE_PURE_NAME(
                void, /* Return type */
                System,      /* Parent class */
                "update",
                Update,
                dt /* Argument(s) */
        );
    }

    void End() override
    {
        PYBIND11_OVERRIDE_PURE_NAME(
                void, /* Return type */
                System,      /* Parent class */
                "end",
                End      /* Argument(s) */
        );
    }
};

class PyManager : public System
{
public:
    void Begin() override;

    void Update(float dt) override;

    void End() override;

    System* LoadScript(std::string_view path, std::string_view className);
private:
    std::vector<py::object> pySystems_;
};
}