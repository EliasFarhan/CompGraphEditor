#include <pybind11/pybind11.h>

PYBIND11_EMBEDDED_MODULE(example, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring

    m.def("add", &add, "A function that adds two numbers");
}