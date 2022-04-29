#include "engine/py_interface.h"
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/operators.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fmt/format.h>

#include "renderer/pipeline.h"
#include "engine/scene.h"
#include "engine/engine.h"
#include "utils/log.h"

PYBIND11_EMBEDDED_MODULE(gpr5300, m)
{
    py::class_<gpr5300::Pipeline>(m, "Pipeline")
            .def("set_float", &gpr5300::Pipeline::SetFloat)
            .def("set_int", &gpr5300::Pipeline::SetInt)
            .def("set_vec2", &gpr5300::Pipeline::SetVec2)
            .def("set_vec3", &gpr5300::Pipeline::SetVec3)
            .def("set_vec4", &gpr5300::Pipeline::SetVec4)
            .def("set_mat4", &gpr5300::Pipeline::SetMat4);

    py::class_<gpr5300::Script, gpr5300::PySystem>(m, "System")
            .def(py::init())
            .def("begin", &gpr5300::Script::Begin)
            .def("update", &gpr5300::Script::Update)
            .def("end", &gpr5300::Script::End)
            .def("draw", &gpr5300::Script::Draw)
    ;

    py::class_<gpr5300::Scene>(m, "Scene")
            .def("get_pipeline", &gpr5300::Scene::GetPipeline,
                 py::return_value_policy::reference)
            .def("get_material", &gpr5300::Scene::GetMaterial);
            

    py::class_<glm::vec2>(m, "Vec2")
        .def(py::init<>())
        .def(py::init<float, float>())
        .def(py::self+py::self)
        .def(py::self+=py::self)
        .def(py::self-py::self)
        .def(-py::self)
        .def(py::self-=py::self)
        .def(py::self*=float())
        .def(py::self*float())
        .def(float()*py::self)
        .def_property_readonly("length", [](glm::vec2 v) {return glm::length(v); })
        .def_static("dot", [](glm::vec2 v1, glm::vec2 v2) {return glm::dot(v1, v2); })

    ;
    py::class_<glm::vec3>(m, "Vec3")
        .def(py::init<>())
        .def(py::init<float, float, float>())
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def(py::self - py::self)
        .def(-py::self)
        .def(py::self -= py::self)
        .def(py::self *= float())
        .def(py::self * float())
        .def(float() * py::self)
        .def_property_readonly("length", [](glm::vec3 v) {return glm::length(v); })
        .def_static("dot", [](glm::vec3 v1, glm::vec3 v2) {return glm::dot(v1, v2); })
        .def_static("cross", [](glm::vec3 v1, glm::vec3 v2) {return glm::cross(v1, v2); })
    ;
    py::class_<glm::vec4>(m, "Vec4")
        .def(py::init<>())
        .def(py::init<float, float, float, float>())
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def(py::self - py::self)
        .def(-py::self)
        .def(py::self -= py::self)
        .def(py::self *= float())
        .def(py::self * float())
        .def(float() * py::self)
        .def_property_readonly("length", [](glm::vec4 v) {return glm::length(v); })
        .def_static("dot", [](glm::vec4 v1, glm::vec4 v2) {return glm::dot(v1, v2); })
        ;
    py::class_<glm::mat3>(m, "Mat3")
        .def(py::init<>())
        .def(py::init<float>())
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def(py::self - py::self)
        .def(-py::self)
        .def(py::self -= py::self)
        .def(py::self *= float())
        .def(py::self * float())
        .def(py::self * py::self)
        .def(float() * py::self)
        .def_property_readonly_static("identity", []() {return glm::mat3(1.0f); })
    ;

    py::class_<glm::mat4>(m, "Mat4")
        .def(py::init<>())
        .def(py::init<float>())
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def(py::self - py::self)
        .def(-py::self)
        .def(py::self -= py::self)
        .def(py::self *= float())
        .def(py::self * float())
        .def(py::self * py::self)
        .def(float() * py::self)
        .def_property_readonly_static("identity", []() {return glm::mat4(1.0f); })
        .def("translate", [](const glm::mat4& mat, glm::vec3 v) {return glm::translate(mat, v); })
        .def("scale", [](const glm::mat4& mat, glm::vec3 v) {return glm::scale(mat, v); })
        .def("rotate", [](const glm::mat4& mat, float radian, glm::vec3 v) {return glm::rotate(mat, radian, v); })
        .def_static("view", [](glm::vec3 eye, glm::vec3 center, glm::vec3 up) {return glm::lookAt(eye, center, up); })
        .def_static("perspective", [](float fovRadian, float aspect, float near, float far) {return glm::perspective(fovRadian, aspect, near, far); })
    ;

    m.def("get_scene", [](){
       return gpr5300::SceneManager::GetInstance()->GetCurrentScene();
    }, py::return_value_policy::reference);

    m.def("get_window_size", []
        {
            return gpr5300::Engine::GetInstance()->GetWindowSize();
        });

    m.def("get_aspect", []
        {
            const auto windowSize = gpr5300::Engine::GetInstance()->GetWindowSize();
            return static_cast<float>(windowSize.x)/static_cast<float>(windowSize.y);
        });

    py::class_<gpr5300::SceneMaterial>(m, "Material")
        .def("bind", &gpr5300::SceneMaterial::Bind)
        .def("get_pipeline", &gpr5300::SceneMaterial::GetPipeline, py::return_value_policy::reference);
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

Script* PyManager::LoadScript(std::string_view path, std::string_view className)
{
    try
    {
        const auto module = py::module_::import(path.data());
        auto newObject = module.attr(className.data())();
        auto* newSystem = newObject.cast<Script*>();
        newSystem->Begin();
        pySystems_.push_back(std::move(newObject));
        return newSystem;
    }
    catch (pybind11::error_already_set& e)
    {
        LogError(fmt::format("{}", e.what()));
        return nullptr;
    }
}
}
