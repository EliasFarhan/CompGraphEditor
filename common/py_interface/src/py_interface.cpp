#include "py_interface.h"
#include "engine/system.h"
#include "engine/scene.h"
#include "renderer/pipeline.h"
#include "engine/filesystem.h"
#include "renderer/command.h"
#include "renderer/framebuffer.h"
#include "renderer/buffer.h"
#include "maths/angle.h"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/operators.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <format>
#include <SDL3/SDL_keycode.h>


struct TypeInfo
{
    std::string_view pyName;
    std::size_t typeSize;
    std::size_t formatSize;
    std::string_view format;
    std::pair<std::size_t, std::size_t> elementCount;
};

TypeInfo GetPyTypeSize(std::string_view typeString)
{
    static constexpr std::array<TypeInfo, 8> types =
    {
        {
            {"neko2.Vec2", sizeof(glm::vec2),sizeof(float), "f",{2,1}},
            {"neko2.Vec3", sizeof(glm::vec3),sizeof(float), "f",{3,1}},
            {"neko2.Vec4", sizeof(glm::vec4),sizeof(float), "f",{4,1}},
            {"neko2.Mat2", sizeof(glm::mat2),sizeof(float), "f",{2, 2}},
            {"neko2.Mat3", sizeof(glm::mat3),sizeof(float), "f",{3, 3}},
            {"neko2.Mat4", sizeof(glm::mat4),sizeof(float), "f",{4, 4}},
            {"float", sizeof(float), sizeof(float), "f", {1, 1}},
            {"int", sizeof(int),sizeof(int), "i", {1, 1}},
        }
    };
    for(const auto& typePair : types)
    {
        if(typeString.find(typePair.pyName) != std::string::npos)
        {
            return typePair;
        }
    }

    return {};
}

core::ArrayBuffer GetArrayBuffer(const core::BufferId& bufferId)
{
    auto& bufferManager = core::GetCurrentScene()->GetBufferManager();
    const auto arrayBuffer = bufferManager.GetArrayBuffer(bufferId);
    return arrayBuffer;
}

PYBIND11_EMBEDDED_MODULE(neko2, m)
{

    py::class_<core::Script, core::PySystem>(m, "System")
        .def(py::init())
        .def("begin", &core::Script::Begin)
        .def("update", &core::Script::Update)
        .def("end", &core::Script::End)
        .def("draw", &core::Script::Draw, py::return_value_policy::reference)
        .def("dispatch", &core::Script::Dispatch, py::return_value_policy::reference)
        .def("on_key_up", &core::Script::OnKeyUp)
        .def("on_key_down", &core::Script::OnKeyDown)
        .def("on_mouse_motion", &core::Script::OnMouseMotion)
    ;

    py::class_<core::Image>(m, "Image");
    py::class_<core::Command>(m, "Command")
        .def("set_float", &core::Command::SetFloat)
        .def("set_int", &core::Command::SetInt)
        .def("set_vec2", &core::Command::SetVec2)
        .def("set_vec3", &core::Command::SetVec3)
        .def("set_vec4", &core::Command::SetVec4)
        .def("set_mat3", &core::Command::SetMat3)
        .def("set_mat4", &core::Command::SetMat4);

    py::class_<core::ComputeCommand>(m, "ComputeCommand")
        .def("set_float", &core::ComputeCommand::SetFloat)
        .def("set_int", &core::ComputeCommand::SetInt)
        .def("set_bool", &core::ComputeCommand::SetBool)
        .def("set_vec2", &core::ComputeCommand::SetVec2)
        .def("set_vec3", &core::ComputeCommand::SetVec3)
        .def("set_vec4", &core::ComputeCommand::SetVec4)
        .def("set_mat3", &core::ComputeCommand::SetMat3)
        .def("set_mat4", &core::ComputeCommand::SetMat4)
        .def("bind", &core::ComputeCommand::Bind)
        .def("dispatch", [](core::ComputeCommand& command, int x, int y, int z)
            {
                auto* scene = core::GetCurrentScene();
                scene->Dispatch(command, x, y, z);
            })
        .def("set_image", [](core::ComputeCommand& command, core::Image* image, int bindingPoint, core::Image::AccessType access)
            {
                if (image)
                {
                    image->BindImage(bindingPoint, access);
                }
                else
                {
                    LogError("Image is null");
                }
            });
    py::class_<core::DrawCommand>(m, "DrawCommand")
        .def("set_float", &core::DrawCommand::SetFloat)
        .def("set_int", &core::DrawCommand::SetInt)
        .def("set_vec2", &core::DrawCommand::SetVec2)
        .def("set_vec3", &core::DrawCommand::SetVec3)
        .def("set_vec4", &core::DrawCommand::SetVec4)
        .def("set_mat3", &core::DrawCommand::SetMat3)
        .def("set_mat4", &core::DrawCommand::SetMat4)
        .def("draw", [](core::DrawCommand& drawCommand)
        {
                auto* scene = core::GetCurrentScene();
                drawCommand.PreDrawBind();
                scene->Draw(drawCommand);
        })
        .def("draw_instanced", [](core::DrawCommand& drawCommand, int instance)
            {
                auto* scene = core::GetCurrentScene();
                drawCommand.PreDrawBind();
                scene->Draw(drawCommand, instance);
            })
        .def("bind", &core::DrawCommand::Bind)
        .def("get_material", [](core::DrawCommand& drawCommand)
        {
            auto* scene = core::GetCurrentScene();
            return scene->GetMaterial(drawCommand.GetMaterialIndex());
        })
        .def("get_name", &core::DrawCommand::GetName)
        .def_property_readonly("name", &core::DrawCommand::GetName)
        .def("get_mesh_name", [](core::DrawCommand& drawCommand)
        {
            auto* scene = core::GetCurrentScene();
            return scene->GetMeshName(drawCommand.GetMeshIndex());
        })
        .def_property_readonly("subpass_index", &core::DrawCommand::GetSubpassIndex)
        .def_property_readonly("mesh_name", &core::DrawCommand::GetName)
        .def_readwrite("model_matrix", &core::DrawCommand::modelTransformMatrix)
    ;


    py::class_<core::SceneSubPass>(m, "SubPass")
        .def("get_draw_command", &core::SceneSubPass::GetDrawCommand)
        .def_property_readonly("draw_command_count", &core::SceneSubPass::GetDrawCommandCount)
        .def("get_framebuffer", &core::SceneSubPass::GetFramebuffer, py::return_value_policy::reference)
        ;

    py::class_<core::Framebuffer>(m, "Framebuffer")
        .def("get_image", &core::Framebuffer::GetImage);

    py::class_<core::Scene>(m, "Scene")
        .def("get_pipeline", &core::Scene::GetPipeline,
            py::return_value_policy::reference)
        .def("get_material", &core::Scene::GetMaterial)
        .def("get_subpass", &core::Scene::GetSubpass)
        .def("get_framebuffer", &core::Scene::GetFramebuffer, py::return_value_policy::reference)
        .def("get_camera", &core::Scene::GetCamera, py::return_value_policy::reference)
        .def_property_readonly("camera", &core::Scene::GetCamera, py::return_value_policy::reference)
        .def_property_readonly("subpass_count", &core::Scene::GetSubpassCount)
        .def_property_readonly("pipeline_count", &core::Scene::GetPipelineCount)
        .def_property_readonly("material_count", &core::Scene::GetMaterialCount)
        ;
            

    py::class_<glm::vec2>(m, "Vec2", py::buffer_protocol())
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
        .def(py::self*float())
        .def(py::self/float())
        .def(float()*py::self)
        .def("normalize", [](const glm::vec2& v) {return glm::normalize(v); })
        .def_property_readonly("length", [](glm::vec2 v) {return glm::length(v); })
        .def_static("dot", [](glm::vec2 v1, glm::vec2 v2) {return glm::dot(v1, v2); })
        .def("__repr__",
            [](const glm::vec2& a) {
                return std::format("({},{})",a.x, a.y);
            })
        .def_buffer([](glm::vec2& m) -> py::buffer_info {
                return py::buffer_info(
                    &m[0],                               /* Pointer to buffer */
                    sizeof(float),                          /* Size of one scalar */
                    py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
                    1,                                      /* Number of dimensions */
                    { 2 },                 /* Buffer dimensions */
                    { sizeof(float) }
                );
            })
    ;
    py::class_<glm::vec3>(m, "Vec3", py::buffer_protocol())

        .def_readwrite("x", &glm::vec3::x)
        .def_readwrite("y", &glm::vec3::y)
        .def_readwrite("z", &glm::vec3::z)
        .def(py::init<>())
        .def(py::init<float, float, float>())
        .def(py::init<float>())
        .def(py::init<const glm::vec2&, float>())
        .def(py::init<const glm::vec4&>())
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def(py::self - py::self)
        .def(-py::self)
        .def(py::self -= py::self)
        .def(py::self *= float())
        .def(py::self * float())
        .def(py::self / float())
        .def(float() * py::self)
        .def("normalize", [](const glm::vec3& v) {return glm::normalize(v); })
        .def_property_readonly("length", [](glm::vec3 v) {return glm::length(v); })
        .def_static("dot", [](glm::vec3 v1, glm::vec3 v2) {return glm::dot(v1, v2); })
        .def_static("cross", [](glm::vec3 v1, glm::vec3 v2) {return glm::cross(v1, v2); })
        .def("__repr__",
                [](const glm::vec3& a) {
                    return std::format("({},{},{})", a.x, a.y, a.z);
                })
        .def_buffer([](glm::vec3& m) -> py::buffer_info {
                return py::buffer_info(
                    &m[0],                               /* Pointer to buffer */
                    sizeof(float),                          /* Size of one scalar */
                    py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
                    1,                                      /* Number of dimensions */
                    { 3 },                 /* Buffer dimensions */
                    { sizeof(float) }
                );
    })
    ;
    py::class_<glm::vec4>(m, "Vec4", py::buffer_protocol())
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
        .def(py::self / float())
        .def(float() * py::self)

        .def("normalize", [](const glm::vec4& v) {return glm::normalize(v); })
        .def_property_readonly("length", [](glm::vec4 v) {return glm::length(v); })
        .def_static("dot", [](glm::vec4 v1, glm::vec4 v2) {return glm::dot(v1, v2); })
        .def("__repr__",
            [](const glm::vec4& a) {
                return std::format("({},{},{},{})", a.x, a.y, a.z, a.w);
            })
        .def_buffer([](glm::vec4& m) -> py::buffer_info {
                return py::buffer_info(
                    &m[0],                               /* Pointer to buffer */
                    sizeof(float),                          /* Size of one scalar */
                    py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
                    1,                                      /* Number of dimensions */
                    { 4 },                 /* Buffer dimensions */
                    { sizeof(float) }
                );
            })
        ;
    py::class_<glm::mat3>(m, "Mat3", py::buffer_protocol())
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
        .def(py::self / float())
        .def(py::self * glm::vec3())
        .def(py::self * py::self)
        .def(float() * py::self)
        .def("inverse", [](const glm::mat3& m) {return glm::inverse(m); })
        .def("transpose", [](const glm::mat3& m) {return glm::transpose(m); })
        .def_buffer([](glm::mat3& m) -> py::buffer_info {
        return py::buffer_info(
            &m[0],                               /* Pointer to buffer */
            sizeof(float),                          /* Size of one scalar */
            py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
            2,                                      /* Number of dimensions */
            { 3,3 },                 /* Buffer dimensions */
            { sizeof(float)*3, sizeof(float)}
        );
            })
    ;

    py::class_<glm::mat4>(m, "Mat4", py::buffer_protocol())
        .def(py::init<>())
        .def(py::init<float>())
        .def(py::init<const glm::mat3&>())
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def(py::self - py::self)
        .def(-py::self)
        .def(py::self -= py::self)
        .def(py::self *= float())
        .def(py::self * float())
        .def(py::self / float())
        .def(py::self * glm::vec4())
        .def(py::self * py::self)
        .def(float() * py::self)
        .def("translate", [](const glm::mat4& mat, glm::vec3 v) {return glm::translate(mat, v); })
        .def("scale", [](const glm::mat4& mat, glm::vec3 v) {return glm::scale(mat, v); })
        .def("rotate", [](const glm::mat4& mat, float radian, glm::vec3 v) {return glm::rotate(mat, radian, v); })
        .def("inverse", [](const glm::mat4& mat) { return glm::inverse(mat); })
        .def("transpose", [](const glm::mat4& mat) { return glm::transpose(mat); })
        .def_static("view", [](glm::vec3 eye, glm::vec3 center, glm::vec3 up) {return glm::lookAt(eye, center, up); })
        .def_static("perspective", [](float fovRadian, float aspect, float near, float far)
        {
            return glm::perspective(fovRadian, aspect, near, far);
        })
        .def_static("orthographic", [](float width, float height, float near, float far)
        {
            return glm::ortho(0.0f, width, 0.0f, height, near, far);
        })
        .def_static("orthographic", [](float left, float right, float bottom, float top, float near, float far)
        {
            return glm::ortho(left, right, bottom, top, near, far);
        })
        .def_buffer([](glm::mat4& m) -> py::buffer_info {
        return py::buffer_info(
            &m[0],                               /* Pointer to buffer */
            sizeof(float),                          /* Size of one scalar */
            py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
            2,                                      /* Number of dimensions */
            { 4,4 },                 /* Buffer dimensions */
            { sizeof(float) * 4, sizeof(float) }
        );
            })
    ;

    m.def("get_scene", []{
       return core::GetCurrentScene();
    }, py::return_value_policy::reference);

    m.def("get_window_size", []
        {
            return glm::vec2{ core::GetWindowSize() };
        });

    m.def("get_aspect", []
        {
            const auto windowSize = core::GetWindowSize();
            return static_cast<float>(windowSize.x)/static_cast<float>(windowSize.y);
        });

    py::class_<core::SceneMaterial>(m, "Material")
        .def("get_pipeline", &core::SceneMaterial::GetPipeline, py::return_value_policy::reference)
        .def("get_name", &core::SceneMaterial::GetName)
        .def_property_readonly("name", &core::SceneMaterial::GetName)
        ;

    py::enum_<core::Image::AccessType>(m, "Access", py::arithmetic())
        .value("READ_ONLY", core::Image::AccessType::READ_ONLY)
        .value("WRITE_ONLY", core::Image::AccessType::WRITE_ONLY)
        .value("READ_WRITE", core::Image::AccessType::READ_WRITE)
    .export_values();

    py::dict keyAttrs;
        keyAttrs["A"] = py::int_(SDLK_A);
        keyAttrs["B"] = py::int_(SDLK_B);
        keyAttrs["C"] = py::int_(SDLK_C);
        keyAttrs["D"] = py::int_(SDLK_D);
        keyAttrs["E"] = py::int_(SDLK_E);
        keyAttrs["F"] = py::int_(SDLK_F);
        keyAttrs["G"] = py::int_(SDLK_G);
        keyAttrs["H"] = py::int_(SDLK_H);
        keyAttrs["I"] = py::int_(SDLK_I);
        keyAttrs["J"] = py::int_(SDLK_J);
        keyAttrs["K"] = py::int_(SDLK_K);
        keyAttrs["L"] = py::int_(SDLK_L);
        keyAttrs["M"] = py::int_(SDLK_M);
        keyAttrs["N"] = py::int_(SDLK_N);
        keyAttrs["O"] = py::int_(SDLK_O);
        keyAttrs["P"] = py::int_(SDLK_P);
        keyAttrs["Q"] = py::int_(SDLK_Q);
        keyAttrs["R"] = py::int_(SDLK_R);
        keyAttrs["S"] = py::int_(SDLK_S);
        keyAttrs["T"] = py::int_(SDLK_T);
        keyAttrs["U"] = py::int_(SDLK_U);
        keyAttrs["V"] = py::int_(SDLK_V);
        keyAttrs["W"] = py::int_(SDLK_W);
        keyAttrs["X"] = py::int_(SDLK_X);
        keyAttrs["Y"] = py::int_(SDLK_Y);
        keyAttrs["Z"] = py::int_(SDLK_Z);
        keyAttrs["BACKSPACE"] =SDLK_BACKSPACE;
        keyAttrs["DELETE"] =SDLK_DELETE;
        keyAttrs["UP"] =SDLK_UP;
        keyAttrs["DOWN"] =SDLK_DOWN;
        keyAttrs["LEFT"] =SDLK_LEFT;
        keyAttrs["RIGHT"] =SDLK_RIGHT;
        keyAttrs["ESCAPE"] =SDLK_ESCAPE;
        keyAttrs["LALT"] =SDLK_LALT;
        keyAttrs["RALT"] =SDLK_RALT;
        keyAttrs["LCTRL"] =SDLK_LCTRL;
        keyAttrs["RCTRL"] =SDLK_RCTRL;
        keyAttrs["LSHIFT"] =SDLK_LSHIFT;
        keyAttrs["RSHIFT"] =SDLK_RSHIFT;
        keyAttrs["SPACE"] =SDLK_SPACE;
        keyAttrs["RETURN"] = SDLK_RETURN;
        ;
    py::object keycodeClass = py::type::of(py::object())("Key", py::make_tuple(), keyAttrs);
    m.attr("Key") = keycodeClass;

    py::enum_<core::Camera::ProjectionType>(m, "ProjectionType")
        .value("PERSPECTIVE", core::Camera::ProjectionType::PERSPECTIVE)
        .value("ORTHOGRAPHIC", core::Camera::ProjectionType::ORTHOGRAPHIC)
        .value("NONE", core::Camera::ProjectionType::NONE)
        .export_values();

    py::class_<core::Camera>(m, "Camera")
        .def(py::init<>())
        .def_readwrite("position", &core::Camera::position, py::return_value_policy::reference)
        .def_readwrite("direction", &core::Camera::direction, py::return_value_policy::reference)
        .def_readwrite("up", &core::Camera::up, py::return_value_policy::reference)
        .def_readwrite("near", &core::Camera::near, py::return_value_policy::reference)
        .def_readwrite("far", &core::Camera::far, py::return_value_policy::reference)
        .def_readwrite("fov", &core::Camera::fovY, py::return_value_policy::reference)
        .def_readwrite("aspect", &core::Camera::aspect, py::return_value_policy::reference)
        .def_readwrite("orthographic_size", &core::Camera::orthographicHalfHeight, py::return_value_policy::reference)
        .def_readwrite("projection_type", &core::Camera::projectionType, py::return_value_policy::reference)
        .def("get_view", &core::Camera::GetView)
        .def("get_projection", &core::Camera::GetProjection)
        .def_property_readonly("view", &core::Camera::GetView)
        .def_property_readonly("projection", &core::Camera::GetProjection)
    ;

    py::class_<core::Pipeline>(m, "Pipeline")
            .def("get_name", &core::Pipeline::GetPipelineName)
            .def_property_readonly("name", &core::Pipeline::GetPipelineName);

    py::class_<core::ModelTransformMatrix>(m, "ModelTransformMatrix")
        .def_property("translate", &core::ModelTransformMatrix::GetTranslate, &core::ModelTransformMatrix::SetTranslate)
        .def_property("scale", &core::ModelTransformMatrix::GetScale, &core::ModelTransformMatrix::SetScale)
        .def_property("rotation", &core::ModelTransformMatrix::GetRotation, &core::ModelTransformMatrix::SetRotation)
        .def_property_readonly("transform", &core::ModelTransformMatrix::GetModelTransformMatrix)
        ;
    py::class_<core::CameraSystem>(m, "CameraSystem")
        .def_readwrite("camera", 
            &core::CameraSystem::camera,
            py::return_value_policy::reference);

    py::class_<core::BufferId>(m, "Buffer", py::buffer_protocol())
        .def("memory_view", [](core::BufferId bufferId, const py::handle& type)
        {
            const auto typeString = py::str(type).cast<std::string>();
            const auto arrayBuffer = GetArrayBuffer( bufferId);
            const auto typeInfo = GetPyTypeSize(typeString);
            if (typeInfo.elementCount.second == 1)
            {
                return py::memoryview::from_buffer(
                    arrayBuffer.data,
                    typeInfo.formatSize,
                    typeInfo.format.data(),
                    { arrayBuffer.count, typeInfo.elementCount.first },
                    { arrayBuffer.typeSize, typeInfo.formatSize }
                );
            }
            else
            {
                return py::memoryview::from_buffer(
                    arrayBuffer.data,
                    typeInfo.formatSize,
                    typeInfo.format.data(),
                    { arrayBuffer.count, typeInfo.elementCount.first, typeInfo.elementCount.second },
                    { arrayBuffer.typeSize, typeInfo.formatSize*typeInfo.elementCount.second }
                );
            }
        })
        .def_buffer([](core::BufferId bufferId)
        {
            const auto arrayBuffer = GetArrayBuffer(bufferId);
            return py::buffer_info(
                arrayBuffer.data,                               /* Pointer to buffer */
                sizeof(float),                          /* Size of one scalar */
                py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
                2,                                      /* Number of dimensions */
                { static_cast<int>(arrayBuffer.count), 4 },                 /* Buffer dimensions */
                { arrayBuffer.typeSize, sizeof(float) }
            );
        })
    ;

    m.def("get_camera_system", []() {
        return core::GetCameraSystem();
        }, py::return_value_policy::reference);
    m.def("get_buffer", [](std::string_view bufferName)
        {
            auto& bufferManager = core::GetCurrentScene()->GetBufferManager();
            return bufferManager.GetBuffer(bufferName);
        });

}



namespace core
{
PyManager::PyManager()
{
    ScriptLoaderLocator::provide(this);
}

void PyManager::Begin()
{
    if(initialized)
    {
        LogError("Python Script Loader should not be initiliazed");
        std::terminate();
    }
    py::initialize_interpreter();
    initialized = true;
}


void PyManager::End()
{
    if (!initialized)
    {
        LogError("Python Script Loader should be initiliazed");
        std::terminate();
    }
    for(auto& object : pySystems_)
    {
        auto& system = object.cast<Script&>();
        system.End();
    }
    pySystems_.clear();
    py::finalize_interpreter();
    initialized = false;
}

Script* PyManager::LoadScript(std::string_view path, std::string_view module, std::string_view className)
{
    auto* nativeScript = MinimalScriptLoader::LoadScript(path, module, className);
    if(nativeScript != nullptr)
    {
        nativeScript->Begin();
        return nativeScript;
    }
    if(!FileExists(path))
    {
        LogError(std::format("Could not find script file at path: {}", path));
        return nullptr;
    }
    try
    {
        const auto moduleObj = py::module_::import(module.data());
        auto newObject = moduleObj.attr(className.data())();
        pySystems_.push_back(std::move(newObject));
        auto* newSystem = pySystems_.back().cast<Script*>();
        newSystem->Begin();
        
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
            LogError(std::format("{}", e.what()));
        //}
        return nullptr;
    }

}

}
