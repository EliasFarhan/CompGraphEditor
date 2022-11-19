//
// Created by unite on 17.11.2022.
//

#include "gl/pipeline.h"
#include "gl/framebuffer.h"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

PYBIND11_EMBEDDED_MODULE(gl, m)
{
    py::class_<gpr5300::gl::Pipeline>(m, "Pipeline")
            .def("set_float", &gpr5300::Pipeline::SetFloat)
            .def("set_int", &gpr5300::Pipeline::SetInt)
            .def("set_vec2", &gpr5300::Pipeline::SetVec2)
            .def("set_vec3", &gpr5300::Pipeline::SetVec3)
            .def("set_vec4", &gpr5300::Pipeline::SetVec4)
            .def("set_mat4", &gpr5300::Pipeline::SetMat4)
            .def("set_texture", [](gpr5300::Pipeline &pipeline,
                    std::string_view uniformName,
                    GLuint textureName,
                    GLenum textureUnit = 0)
                {
                        return pipeline.SetTexture(uniformName, textureName, textureUnit);
            })
            .def("set_cubemap", [](gpr5300::Pipeline &pipeline,
                    std::string_view uniformName,
                    GLuint textureName,
                    GLenum textureUnit = 0) {
                return pipeline.SetCubemap(uniformName, textureName, textureUnit);
            })
            .def("get_name", &gpr5300::Pipeline::GetPipelineName)
            .def_property_readonly("name", &gpr5300::Pipeline::GetPipelineName);


    py::class_<gpr5300::Framebuffer>(m, "Framebuffer")
        .def("get_texture_name", &gpr5300::Framebuffer::GetTextureName)
        .def("get_name", &gpr5300::Framebuffer::GetName)
        .def_property_readonly("name", &gpr5300::Framebuffer::GetName);
}