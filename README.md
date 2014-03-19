# OCCA
====

You can find our pre-print in [arXiv](http://arxiv.org/abs/1403.0968)

## Building the C and Fortran Interfaces with CMake

To build this code using the defaults run

    > cmake .
    > make

A debug version of the code can be built by passing an option to CMake

    > cmake -DCMAKE_BUILD_TYPE="Debug" .
    > make

By default CMake tries to guess your compiler.  You can tell CMake which
compilers to use by passing in command line arguments

    > cmake -DCMAKE_C_COMPILER="gcc-4.8.1" \
            -DCMAKE_CXX_COMPILER="g++-4.8.1" \
            -DCMAKE_Fortran_COMPILER="gfortran-4.8.1" \
            .
    > make

By default all available back-ends are built.  They can be turned off by
passing options to CMake

    > cmake -DWITH_CUDA=OFF -DWITH_CPU=OFF -DWITH_OPENCL=OFF
    > make
