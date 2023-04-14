
add_library(CommonPy STATIC include/py_interface.h src/py_interface.cpp)
target_include_directories(CommonPy PUBLIC include/ ${Python3_INCLUDE_DIRS})
target_link_libraries(CommonPy PUBLIC Core pybind11::embed ${Python3_LIBRARIES})
set_target_properties (CommonPy PROPERTIES FOLDER Core)
