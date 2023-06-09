
add_subdirectory(imnodes)
add_subdirectory(pybind11)
add_subdirectory(basis_universal)
set_target_properties (basisu PROPERTIES FOLDER External)


if(ENABLE_PROFILER)
    add_subdirectory(tracy)

    set_target_properties (TracyClient PROPERTIES FOLDER External)
endif()