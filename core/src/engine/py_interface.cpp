#include "engine/py_interface.h"
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/operators.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fmt/format.h>

#include "engine/filesystem.h"
#include "engine/scene.h"
#include "engine/engine.h"
#include "utils/log.h"

PYBIND11_EMBEDDED_MODULE(gpr5300, m)
{
    

    py::class_<gpr5300::Script, gpr5300::PySystem>(m, "System")
            .def(py::init())
            .def("begin", &gpr5300::Script::Begin)
            .def("update", &gpr5300::Script::Update)
            .def("end", &gpr5300::Script::End)
            .def("draw", &gpr5300::Script::Draw)
            .def("on_key_up", &gpr5300::Script::OnKeyUp)
            .def("on_key_down", &gpr5300::Script::OnKeyDown)
            .def("on_mouse_motion", &gpr5300::Script::OnMouseMotion)
    ;


    py::class_<gpr5300::SceneSubPass>(m, "SubPass")
        .def("get_draw_command", &gpr5300::SceneSubPass::GetDrawCommand)
        .def_property_readonly("draw_command_count", &gpr5300::SceneSubPass::GetDrawCommandCount)
        .def("get_framebuffer", &gpr5300::SceneSubPass::GetFramebuffer, py::return_value_policy::reference)
        ;

    py::class_<gpr5300::Scene>(m, "Scene")
            .def("get_pipeline", &gpr5300::Scene::GetPipeline,
                 py::return_value_policy::reference)
            .def("get_material", &gpr5300::Scene::GetMaterial)
            .def("get_subpass", &gpr5300::Scene::GetSubpass)
            .def_property_readonly("subpass_count", &gpr5300::Scene::GetSubpassCount)
            .def_property_readonly("pipeline_count", &gpr5300::Scene::GetPipelineCount)
            .def_property_readonly("material_count", &gpr5300::Scene::GetMaterialCount)
            ;
            

    py::class_<glm::vec2>(m, "Vec2")
        .def_readwrite("x", &glm::vec2::x)
        .def_readwrite("y", &glm::vec2::y)
        .def(py::init<>())
        .def(py::init<float, float>())
        .def(py::init<const glm::vec3&>())
        .def(py::init<const glm::vec4&>())
        .def(py::self+py::self)
        .def(py::self+=py::self)
        .def(py::self-py::self)
        .def(-py::self)
        .def(py::self-=py::self)
        .def(py::self*=float())
        .def(py::self*float())
        .def(float()*py::self)

        .def("normalize", [](const glm::vec2& v) {return glm::normalize(v); })
        .def_property_readonly("length", [](glm::vec2 v) {return glm::length(v); })
        .def_static("dot", [](glm::vec2 v1, glm::vec2 v2) {return glm::dot(v1, v2); })
        .def("__repr__",
            [](const glm::vec2& a) {
                return fmt::format("({},{})",a.x, a.y);
            })
    ;
    py::class_<glm::vec3>(m, "Vec3")

        .def_readwrite("x", &glm::vec3::x)
        .def_readwrite("y", &glm::vec3::y)
        .def_readwrite("z", &glm::vec3::z)
        .def(py::init<>())
        .def(py::init<float, float, float>())
        .def(py::init<const glm::vec2&, float>())
        .def(py::init<const glm::vec4&>())
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def(py::self - py::self)
        .def(-py::self)
        .def(py::self -= py::self)
        .def(py::self *= float())
        .def(py::self * float())
        .def(float() * py::self)
        .def("normalize", [](const glm::vec3& v) {return glm::normalize(v); })
        .def_property_readonly("length", [](glm::vec3 v) {return glm::length(v); })
        .def_static("dot", [](glm::vec3 v1, glm::vec3 v2) {return glm::dot(v1, v2); })
        .def_static("cross", [](glm::vec3 v1, glm::vec3 v2) {return glm::cross(v1, v2); })
    .def("__repr__",
            [](const glm::vec3& a) {
                return fmt::format("({},{},{})", a.x, a.y, a.z);
            })
    ;
    py::class_<glm::vec4>(m, "Vec4")
        .def_readwrite("x", &glm::vec4::x)
        .def_readwrite("y", &glm::vec4::y)
        .def_readwrite("z", &glm::vec4::z)
        .def_readwrite("w", &glm::vec4::w)
        .def(py::init<>())
        .def(py::init<float>())
        .def(py::init<const glm::vec2&, float, float>())
        .def(py::init<const glm::vec3&, float>())
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def(py::self - py::self)
        .def(-py::self)
        .def(py::self -= py::self)
        .def(py::self *= float())
        .def(py::self * float())
        .def(float() * py::self)

        .def("normalize", [](const glm::vec4& v) {return glm::normalize(v); })
        .def_property_readonly("length", [](glm::vec4 v) {return glm::length(v); })
        .def_static("dot", [](glm::vec4 v1, glm::vec4 v2) {return glm::dot(v1, v2); })
        .def("__repr__",
            [](const glm::vec4& a) {
                return fmt::format("({},{},{},{})", a.x, a.y, a.z, a.w);
            })
        ;
    py::class_<glm::mat3>(m, "Mat3")
        .def(py::init<>())
        .def(py::init<float>())
        .def(py::init<const glm::mat4&>())
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def(py::self - py::self)
        .def(-py::self)
        .def(py::self -= py::self)
        .def(py::self *= float())
        .def(py::self * float())
        .def(py::self* glm::vec3())
        .def(py::self * py::self)
        .def(float() * py::self)
        .def("inverse", [](const glm::mat3& m) {return glm::inverse(m); })
        .def("transpose", [](const glm::mat3& m) {return glm::transpose(m); })
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
        .def(py::self * glm::vec4())
        .def(py::self * py::self)
        .def(float() * py::self)
        .def("translate", [](const glm::mat4& mat, glm::vec3 v) {return glm::translate(mat, v); })
        .def("scale", [](const glm::mat4& mat, glm::vec3 v) {return glm::scale(mat, v); })
        .def("rotate", [](const glm::mat4& mat, float radian, glm::vec3 v) {return glm::rotate(mat, radian, v); })
        .def("inverse", [](const glm::mat4& mat) { return glm::inverse(mat); })
        .def("transpose", [](const glm::mat4& mat) { return glm::transpose(mat); })
        .def_static("view", [](glm::vec3 eye, glm::vec3 center, glm::vec3 up) {return glm::lookAt(eye, center, up); })
        .def_static("perspective", [](float fovRadian, float aspect, float near, float far) {return glm::perspective(fovRadian, aspect, near, far); })
    ;

    m.def("get_scene", [](){
       return gpr5300::GetCurrentScene();
    }, py::return_value_policy::reference);

    m.def("get_window_size", []
        {
            return glm::vec2{ gpr5300::GetWindowSize() };
        });

    m.def("get_aspect", []
        {
            const auto windowSize = gpr5300::GetWindowSize();
            return static_cast<float>(windowSize.x)/static_cast<float>(windowSize.y);
        });

    py::class_<gpr5300::SceneMaterial>(m, "Material")
        .def("bind", &gpr5300::SceneMaterial::Bind)
        .def("get_pipeline", &gpr5300::SceneMaterial::GetPipeline, py::return_value_policy::reference)
        .def("get_name", &gpr5300::SceneMaterial::GetName)
        .def_property_readonly("name", &gpr5300::SceneMaterial::GetName)
        ;

    py::class_<gpr5300::SceneDrawCommand>(m, "DrawCommand")
        .def("draw", &gpr5300::SceneDrawCommand::Draw)
        .def("get_material", &gpr5300::SceneDrawCommand::GetMaterial)
        .def("get_name", &gpr5300::SceneDrawCommand::GetName)
        .def_property_readonly("name", &gpr5300::SceneDrawCommand::GetName)
        .def("get_mesh_name", &gpr5300::SceneDrawCommand::GetMeshName)
        .def_property_readonly("mesh_name", &gpr5300::SceneDrawCommand::GetName)
        ;
    py::enum_<SDL_KeyCode>(m, "Key", py::arithmetic())
        .value("A", SDLK_a)
        .value("B", SDLK_b)
        .value("C", SDLK_c)
        .value("D", SDLK_d)
        .value("E", SDLK_e)
        .value("F", SDLK_f)
        .value("G", SDLK_g)
        .value("H", SDLK_h)
        .value("I", SDLK_i)
        .value("J", SDLK_j)
        .value("K", SDLK_k)
        .value("L", SDLK_l)
        .value("M", SDLK_m)
        .value("N", SDLK_n)
        .value("O", SDLK_o)
        .value("P", SDLK_p)
        .value("Q", SDLK_q)
        .value("R", SDLK_r)
        .value("S", SDLK_s)
        .value("T", SDLK_t)
        .value("U", SDLK_u)
        .value("V", SDLK_v)
        .value("W", SDLK_w)
        .value("X", SDLK_x)
        .value("Y", SDLK_y)
        .value("Z", SDLK_z)
        .value("BACKSPACE", SDLK_BACKSPACE)
        .value("DELETE", SDLK_DELETE)
        .value("UP", SDLK_UP)
        .value("DOWN", SDLK_DOWN)
        .value("LEFT", SDLK_LEFT)
        .value("RIGHT", SDLK_RIGHT)
        .value("ESCAPE", SDLK_ESCAPE)
        .value("LALT", SDLK_LALT)
        .value("RALT", SDLK_RALT)
        .value("LCTRL", SDLK_LCTRL)
        .value("RCTRL", SDLK_RCTRL)
        .value("LSHIFT", SDLK_LSHIFT)
        .value("RSHIFT", SDLK_RSHIFT)
        .value("SPACE", SDLK_SPACE)
        .value("RETURN", SDLK_RETURN)
    .export_values()
        ;
}

namespace gpr5300
{


void PyManager::Begin()
{
    py::initialize_interpreter();
    //
}

void PyManager::Update(float dt)
{
    for(auto& object : pySystems_)
    {
        auto& system = object.cast<Script&>();
        system.Update(dt);
    }
}

void PyManager::End()
{
    for(auto& object : pySystems_)
    {
        auto& system = object.cast<Script&>();
        system.End();
    }
    pySystems_.clear();
    py::finalize_interpreter();
}

Script* PyManager::LoadScript(std::string_view path, std::string_view module, std::string_view className)
{
    const auto& filesystem = FilesystemLocator::get();
    if(!filesystem.FileExists(path))
    {
        LogError(fmt::format("Could not find script file at path: {}", path));
        return nullptr;
    }
    try
    {
        const auto moduleObj = py::module_::import(module.data());
        auto newObject = moduleObj.attr(className.data())();
        auto* newSystem = newObject.cast<Script*>();
        newSystem->Begin();
        pySystems_.push_back(std::move(newObject));
        return newSystem;
    }
    catch (pybind11::error_already_set& e)
    {
        /*try
        {
            const auto scriptFile = filesystem.LoadFile(path);
            const auto locals = py::dict();
            py::exec(reinterpret_cast<const char*>(scriptFile.data), 
                py::globals(), 
                locals);
            const auto classObject = locals[className.data()];
            auto newObject = classObject();
            auto* newSystem = newObject.cast<Script*>();
            pySystems_.push_back(std::move(newObject));
            newSystem->Begin();
            return newSystem;
        }
        catch(pybind11::error_already_set& e)
        {*/
            LogError(fmt::format("{}", e.what()));
        //}
        return nullptr;
    }

}
}
