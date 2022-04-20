#include "engine/py_interface.h"
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "renderer/pipeline.h"
#include "engine/scene.h"

PYBIND11_EMBEDDED_MODULE(gpr5300, m)
{
    py::class_<gpr5300::Pipeline>(m, "Pipeline")
            .def("set_float", &gpr5300::Pipeline::SetFloat);

    py::class_<gpr5300::System, gpr5300::PySystem>(m, "System")
            .def(py::init())
            .def("begin", &gpr5300::System::Begin)
            .def("update", &gpr5300::System::Update)
            .def("end", &gpr5300::System::End);

    py::class_<gpr5300::Scene>(m, "Scene")
            .def("get_pipeline", &gpr5300::Scene::GetPipeline,
                 py::return_value_policy::reference);

    m.def("get_scene", [](){
       return gpr5300::SceneManager::GetInstance()->GetCurrentScene();
    }, py::return_value_policy::reference);
}

namespace gpr5300
{


void PyManager::Begin()
{
    py::initialize_interpreter();
    //auto module = py::module_::import("gpr5300");
}

void PyManager::Update(float dt)
{
    for(auto& object : pySystems_)
    {
        auto& system = object.cast<System&>();
        system.Update(dt);
    }
}

void PyManager::End()
{
    for(auto& object : pySystems_)
    {
        auto& system = object.cast<System&>();
        system.End();
    }
    pySystems_.clear();
    py::finalize_interpreter();
}

System* PyManager::LoadScript(std::string_view path, std::string_view className)
{
    auto module = py::module_::import(path.data());
    auto newObject = module.attr(className.data())();
    auto* newSystem = newObject.cast<System*>();
    newSystem->Begin();
    pySystems_.push_back(std::move(newObject));
    return newSystem;
}
}
