#include "gl/pipeline.h"
#include "gl/framebuffer.h"

#include <engine/py_interface.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

/*
PYBIND11_EMBEDDED_MODULE(gl, m)
{
    
    py::class_<gl::Framebuffer>(m, "Framebuffer")
        .def("get_texture_name", &gl::Framebuffer::GetTextureName)
        .def("get_name", &gl::Framebuffer::GetName)
        .def_property_readonly("name", &gl::Framebuffer::GetName);
}
*/