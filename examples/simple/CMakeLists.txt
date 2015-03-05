find_package(Librt)

configure_file(simple.occa simple.occa COPYONLY)

add_executable(simple simple.cpp)
target_link_libraries(simple occa rt)

add_executable(simple_c simple.c)
target_link_libraries(simple_c occa rt)

add_executable(simple_f90 simple.f90)
target_link_libraries(simple_f90 occa rt)
set_target_properties(simple_f90 PROPERTIES LINKER_LANGUAGE Fortran)
