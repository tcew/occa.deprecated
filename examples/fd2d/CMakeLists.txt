find_package(Librt)
configure_file(setuprc             setuprc COPYONLY)
configure_file(fd2d.occa           fd2d.occa COPYONLY)
configure_file(fd2d_nonShared.occa fd2d_nonShared.occa COPYONLY)
configure_file(fd2d_shared.occa    fd2d_shared.occa COPYONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(fd2d main.cpp src/setupAide.cpp src/visualizer.cpp)
target_link_libraries(fd2d occa rt)

set_target_properties(fd2d PROPERTIES
          COMPILE_DEFINITIONS "datafloat=float")
