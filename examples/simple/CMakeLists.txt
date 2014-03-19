configure_file(simple.occa simple.occa COPYONLY)

add_executable(simple simple.cpp)
target_link_libraries(simple occa)

add_executable(simple_c simple.c)
target_link_libraries(simple_c occa)

add_executable(simple_f90 simple.f90)
target_link_libraries(simple_f90 occa)
set_target_properties(simple_f90 PROPERTIES LINKER_LANGUAGE Fortran)
