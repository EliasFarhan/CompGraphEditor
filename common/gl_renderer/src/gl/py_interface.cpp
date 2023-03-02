#include "gl/pipeline.h"
#include "gl/framebuffer.h"

#include <engine/py_interface.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

PYBIND11_EMBEDDED_MODULE(gl, m)
{
    py::class_<gl::Pipeline>(m, "Pipeline")
            .def("set_float", &gl::Pipeline::SetFloat)
            .def("set_int", &gl::Pipeline::SetInt)
            .def("set_vec2", &gl::Pipeline::SetVec2)
            .def("set_vec3", &gl::Pipeline::SetVec3)
            .def("set_vec4", &gl::Pipeline::SetVec4)
            .def("set_mat4", &gl::Pipeline::SetMat4)
            .def("set_texture", [](gl::Pipeline &pipeline,
                    std::string_view uniformName,
                    GLuint textureName,
                    GLenum textureUnit = 0)
                {
                        return pipeline.SetTexture(uniformName, textureName, textureUnit);
            })
            .def("set_cubemap", [](gl::Pipeline &pipeline,
                    std::string_view uniformName,
                    GLuint textureName,
                    GLenum textureUnit = 0) {
                return pipeline.SetCubemap(uniformName, textureName, textureUnit);
            })
            .def("get_name", &gl::Pipeline::GetPipelineName)
            .def_property_readonly("name", &gl::Pipeline::GetPipelineName);


    py::class_<gl::Framebuffer>(m, "Framebuffer")
        .def("get_texture_name", &gl::Framebuffer::GetTextureName)
        .def("get_name", &gl::Framebuffer::GetName)
        .def_property_readonly("name", &gl::Framebuffer::GetName);
}