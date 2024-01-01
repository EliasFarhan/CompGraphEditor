
add_subdirectory(imnodes)
add_subdirectory(pybind11)


if(ENABLE_PROFILER)
    add_subdirectory(tracy)
    set_target_properties (TracyClient PROPERTIES FOLDER External)
endif()