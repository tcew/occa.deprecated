cmake_minimum_required(VERSION 2.8)

project(OCCA CXX C Fortran)

include(InstallRequiredSystemLibraries)
include(FortranCInterface)
include(CheckIncludeFileCXX)
include(CheckCXXSymbolExists)
include(CMakePushCheckState)

# build options : default all ON
option(WITH_CPU    "Build CPU    back-end?" ON)
option(WITH_OPENCL "Build OPENCL back-end?" ON)
option(WITH_CUDA   "Build CUDA   back-end?" ON)

# Directory where customize cmake files reside
set(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake/findopencl ${CMAKE_MODULE_PATH}
)

find_package(Git)
find_package(OpenMP)

if(WITH_OPENCL)
  find_package(OpenCL)
endif(WITH_OPENCL)

if(WITH_CUDA)
  find_package(CUDA)
endif(WITH_CUDA)

##
## Customize CMake defaults
##
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/local" CACHE PATH
    "OCCA install prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

add_executable(occac_header_gen liboccac/occac_header_gen.c)

function(FILE2HEADER FILE HEADER)
  add_custom_command (
    OUTPUT ${HEADER}
    COMMAND occac_header_gen ${FILE} ${HEADER}
    DEPENDS occac_header_gen ${FILE}
    )
endfunction()

##
## Note debug mode
##
if("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  set(OCCA_DEBUG TRUE)
  add_definitions( -DOCCA_DEBUG=1 )
else("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  set(OCCA_DEBUG FALSE)
endif("${CMAKE_BUILD_TYPE}" MATCHES "Debug")

##
## OS
##
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions( -DOS_OSX=1 )
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions( -DOS_LINUX=1 )
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_definitions( -DOS_WINDOWS=1 )
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")


##
## CPU
##
option(WITH_CPU "Compile with CPU backend" ON)
if(WITH_CPU)
  set(OCCA_USE_CPU ON)
  add_definitions( -DOCCA_USE_CPU=1 )
else(WITH_CPU)
  add_definitions( -DOCCA_USE_CPU=0 )
endif(WITH_CPU)
if(OPENMP_FOUND)
  set(OCCA_HAVE_OPENMP ON)
else(OPENMP_FOUND)
  add_definitions( -DOCCA_DO_NOT_USE_OPENMP )
endif(OPENMP_FOUND)


##
## OpenCL
##
option(WITH_OPENCL "Compile with OpenCL backend" ON)
if(WITH_OPENCL)
  if(OPENCL_FOUND)
    set(OCCA_HAVE_OPENCL ON)
    set(OCCA_USE_OPENCL ON)
    add_definitions( -DOCCA_USE_OPENCL=1 )
    include_directories(${OPENCL_INCLUDE_DIRS})
  else(OPENCL_FOUND)
    message("Warning: Not building the OpenCL backend")
    add_definitions( -DOCCA_USE_OPENCL=0 )
  endif(OPENCL_FOUND)
else(WITH_OPENCL)
    add_definitions( -DOCCA_USE_OPENCL=0 )
endif(WITH_OPENCL)

##
## CUDA
##
option(WITH_CUDA "Compile with CUDA backend" ON)
if(WITH_CUDA)
  if(CUDA_FOUND)
    set(OCCA_HAVE_CUDA ON)
    set(OCCA_USE_CUDA ON)
    add_definitions( -DOCCA_USE_CUDA=1 )
    include_directories(${CUDA_INCLUDE_DIRS})
  else(CUDA_FOUND)
    message("Warning: Not building the CUDA backend")
    add_definitions( -DOCCA_USE_CUDA=0 )
  endif(CUDA_FOUND)
endif(WITH_CUDA)

if(OCCA_USE_OPENCL AND OCCA_USE_CUDA)
  set(OCCA_USE_ALL ON)
  add_definitions( -DOCCA_USE_ALL=1 )
else(OCCA_USE_OPENCL AND OCCA_USE_CUDA)
  add_definitions( -DOCCA_USE_ALL=0 )
endif(OCCA_USE_OPENCL AND OCCA_USE_CUDA)

##
## Set compiler flags
##

## Specify c99 compilers
if("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
if("${CMAKE_C_COMPILER_ID}" MATCHES "Intel")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif("${CMAKE_C_COMPILER_ID}" MATCHES "Intel")
if("${CMAKE_C_COMPILER_ID}" MATCHES "PGI")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -c99")
endif("${CMAKE_C_COMPILER_ID}" MATCHES "PGI")

## Enhance debug flags
if("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-common")
  set(CMAKE_C_FLAGS_DEBUG "-g3")
  set(CMAKE_C_FLAGS_RELEASE "-g0 -O2 -DNDEBUG")
endif("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-common")
  set(CMAKE_C_FLAGS_DEBUG "-g3")
  set(CMAKE_C_FLAGS_RELEASE "-g0 -O2 -DNDEBUG")
endif("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
if("${CMAKE_C_COMPILER_ID}" MATCHES "Intel")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-common")
  set(CMAKE_C_FLAGS_DEBUG "-g3")
  set(CMAKE_C_FLAGS_RELEASE "-g0 -O2 -DNDEBUG")
endif("${CMAKE_C_COMPILER_ID}" MATCHES "Intel")

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common")
  set(CMAKE_CXX_FLAGS_DEBUG "-g3")
  set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O2 -DNDEBUG")
endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common")
  set(CMAKE_CXX_FLAGS_DEBUG "-g3")
  set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O2 -DNDEBUG")
endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common")
  set(CMAKE_CXX_FLAGS_DEBUG "-g3")
  set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O2 -DNDEBUG")
endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")


## Enhance stack trace message
if(LINUX)
  if("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -rdynamic")
  endif("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
  if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -rdynamic")
  endif("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
  if("${CMAKE_C_COMPILER_ID}" MATCHES "Intel")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -rdynamic")
  endif("${CMAKE_C_COMPILER_ID}" MATCHES "Intel")

  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
endif(LINUX)

option(WITH_WARNINGS "Compile with warnings" ON)
if(WITH_WARNINGS)
  # enable compiler warning flags
  if("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -Wno-long-long -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -pedantic -Wextra")
  endif("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
  if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wextra -Wno-unused-parameter -Wno-long-long")
  endif("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
  if("${CMAKE_C_COMPILER_ID}" MATCHES "Intel")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -w2 -Wcheck")
  endif("${CMAKE_C_COMPILER_ID}" MATCHES "Intel")
  if ("${CMAKE_C_COMPILER_ID}" MATCHES "PGI")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Minform=inform")
  endif("${CMAKE_C_COMPILER_ID}" MATCHES "PGI")

  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-long-long -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -pedantic -Wextra")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wextra -Wno-unused-parameter -Wno-long-long")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -w2 -Wcheck")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Minform=inform")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
endif(WITH_WARNINGS)

# Set command for forcing .occa to be c++ files
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -x c++")
endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -x c++")
endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
  set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -x c++")
endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
  set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -cppsuffix=occa")
endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")

option(WITH_CPU_NO_WARNINGS "Compile cpu kernels with no warnings" ON)
if(WITH_CPU_NO_WARNINGS)
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -w")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -w")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -w")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -Minform=fatal")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
endif(WITH_CPU_NO_WARNINGS)

# Turn on vectorization flags
option(WITH_CPU_VECTORIZE "Compile cpu kernels with vectorization" ON)
if(WITH_CPU_VECTORIZE)
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -ftree-vectorize")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -fslp-vectorize-aggressive")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS}")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -Mvect")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
else(WITH_CPU_VECTORIZE)
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -fno-vectorize")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -no-vec")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
endif(WITH_CPU_VECTORIZE)

option(WITH_CPU_RESTRICT "Compile cpu kernels with restrict" ON)
if(WITH_CPU_RESTRICT)
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -restrict")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
endif(WITH_CPU_RESTRICT)

option(WITH_CPU_NATIVE "Compile cpu kernels with native optimizations" ON)
if(WITH_CPU_NATIVE)
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -march=native -mtune=native")
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      # To be able to run with native instructions we need to use the Clang linker.
      # See http://stackoverflow.com/questions/9840207/how-to-use-avx-pclmulqdq-on-mac-os-x-lion
      # for more detials.
      set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -Wa,-q")
    endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -march=native -mtune=native")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -xHost")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS}")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
endif(WITH_CPU_NATIVE)

option(WITH_CPU_UNROLLING "Compile cpu kernels with unrolling" ON)
if(WITH_CPU_UNROLLING)
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -funroll-loops -fsplit-ivs-in-unroller")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -funroll-loops")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -funroll-all-loops")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -Munroll")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
endif(WITH_CPU_UNROLLING)

option(WITH_CPU_FASTMATH "Compile cpu kernels with fast math" ON)
if(WITH_CPU_FASTMATH)
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -ffast-math")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -ffast-math")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -ffast-math")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -fast")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
endif(WITH_CPU_FASTMATH)

option(WITH_CPU_VECTORIZE_VERBOSE "Compile cpu kernels with info about vectorization" ON)
if(WITH_CPU_VECTORIZE_VERBOSE)
  # enable verbose vector flags
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -ftree-vectorizer-verbose=4")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -vec-report3")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
    set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -Minfo=all")
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "PGI")
endif(WITH_CPU_VECTORIZE_VERBOSE)

option(WITH_CPU_O3 "Compile cpu kernels with optimization" ON)
if(WITH_CPU_O3)
  set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -O3")
endif(WITH_CPU_O3)

option(WITH_CPU_DEBUG_SYMBOLS "Compile cpu kernels with debug symbols" ON)
if(WITH_CPU_DEBUG_SYMBOLS)
  set(OCCA_CXX_FLAGS "${OCCA_CXX_FLAGS} -g")
endif(WITH_CPU_DEBUG_SYMBOLS)

set(OCCA_CPU_COMPILER "${CMAKE_CXX_COMPILER}")
set(OCCA_CPU_FLAGS "${OCCA_CXX_FLAGS} ${CMAKE_CXX_COMPILE_OPTIONS_PIC} ${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

set(OCCA_CU_COMPILER "${CUDA_NVCC_EXECUTABLE}")
set(OCCA_CU_FLAGS "-ptx --compiler-options -O3 --use_fast_math")

set(OCCA_CL_FLAGS "-cl-denorms-are-zero -cl-fast-relaxed-math -cl-finite-math-only -cl-mad-enable -cl-no-signed-zeros")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  message("64 bits compiler detected")
  set(EX_PLATFORM 64)
else(CMAKE_SIZEOF_VOID_P EQUAL 8)
  message("32 bits compiler detected")
  set(EX_PLATFORM 32)
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

if(EX_PLATFORM EQUAL 64)
  set(OCCA_CU_FLAGS "-m64 ${OCCA_CU_FLAGS}")
else(EX_PLATFORM EQUAL 64)
  set(OCCA_CU_FLAGS "-m32 ${OCCA_CU_FLAGS}")
endif(EX_PLATFORM EQUAL 64)


##
## Add Compiler info macro definitions
##
add_definitions( -DOCCA_CL_FLAGS="${OCCA_CL_FLAGS}")

add_definitions( -DOCCA_CPU_COMPILER="${OCCA_CPU_COMPILER}")
add_definitions( -DOCCA_CPU_FLAGS="${OCCA_CPU_FLAGS}")

add_definitions( -DOCCA_CU_COMPILER="${OCCA_CU_COMPILER}")
add_definitions( -DOCCA_CU_FLAGS="${OCCA_CU_FLAGS}")

##
## Look for header files
##
# check_include_file_cxx(time.h OCCA_HAVE_TIME_H)
# check_include_file_cxx(sys/time.h OCCA_HAVE_SYS_TIME_H)
# check_include_file_cxx(mach/mach_time.h OCCA_HAVE_MACH_MACH_TIME_H)
#
# cmake_push_check_state()
# if(${APPLE})
# else(${APPLE})
#   set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -D_XOPEN_SOURCE=600)
# endif(${APPLE})
# check_cxx_symbol_exists(CLOCK_MONOTONIC "time.h;sys/time.h" OCCA_HAVE_CLOCK_MONOTONIC)
# check_cxx_symbol_exists(gettimeofday "time.h;sys/time.h" OCCA_HAVE_GETTIMEOFDAY)
# cmake_pop_check_state()
if(UNIX AND NOT APPLE)
  set(TIME_LIBRARIES rt)
endif()
#
# check_symbol_exists(mach_absolute_time "mach/mach_time.h" OCCA_HAVE_MACH_ABSOLUTE_TIME)

##
## Set build include path
##
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libocca)
include_directories(${CMAKE_SOURCE_DIR}/liboccac)
include_directories(${CMAKE_SOURCE_DIR}/liboccaf)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR}/occa)


##
## Version Stuff
##
if(GIT_FOUND)
  set(USE_GIT 1)
else(GIT_FOUND)
  set(USE_GIT 0)
endif(GIT_FOUND)
file(WRITE ${CMAKE_BINARY_DIR}/Version.cmake
"if(${USE_GIT} EQUAL 1)
  execute_process(
       COMMAND ${GIT_EXECUTABLE} describe --dirty ${GIT_DESCRIBE_FLAGS}
       WORKING_DIRECTORY \${WORKING_DIRECTORY}
       OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
       RESULT_VARIABLE GIT_DESCRIBE_RESULT
       ERROR_VARIABLE GIT_DESCRIBE_ERROR
       OUTPUT_STRIP_TRAILING_WHITESPACE
   )
else(${USE_GIT} EQUAL 1)
  set(GIT_DESCRIBE_RESULT -1)
endif(${USE_GIT} EQUAL 1)

if(GIT_DESCRIBE_RESULT EQUAL 0)
  file(WRITE ${CMAKE_BINARY_DIR}/source/VERSION \"\${GIT_DESCRIBE_VERSION}\")
else(GIT_DESCRIBE_RESULT EQUAL 0)
  file(READ  ${CMAKE_SOURCE_DIR}/VERSION GIT_DESCRIBE_VERSION)
endif(GIT_DESCRIBE_RESULT EQUAL 0)

string(REGEX REPLACE \"v([0-9]*)\\\\.([0-9]*)\\\\.(.*)\"
       \"\\\\1.\\\\2.\\\\3\" VERSION \"\${GIT_DESCRIBE_VERSION}\" )

string(REGEX REPLACE \"v([0-9]*)\\\\.([0-9]*)\\\\.(.*)\"
       \"\\\\1\" VERSION_MAJOR \"\${GIT_DESCRIBE_VERSION}\")
string(REGEX REPLACE \"v([0-9]*)\\\\.([0-9]*)\\\\.(.*)\"
       \"\\\\2\" VERSION_MINOR \"\${GIT_DESCRIBE_VERSION}\")
string(REGEX REPLACE \"v([0-9]*)\\\\.([0-9]*)\\\\.(.*)\"
       \"\\\\3\" VERSION_PATCH \"\${GIT_DESCRIBE_VERSION}\")
")

file(WRITE ${CMAKE_BINARY_DIR}/FillVersion.cmake
"INCLUDE(Version.cmake)
CONFIGURE_FILE(\${SRC} \${DST} @ONLY)
")


file(WRITE ${CMAKE_BINARY_DIR}/occa/occa_version_info.h.in
  "\#ifndef OCCA_VERSION_INFO_H\n"
  "\#define OCCA_VERSION_INFO_H\n"
  "\#define OCCA_VERSION_NUMBER \"@VERSION@\"\n"
  "\#endif\n"
)
add_custom_target(
    version
    ${CMAKE_COMMAND} -D SRC=${CMAKE_BINARY_DIR}/occa/occa_version_info.h.in
                     -D DST=${CMAKE_BINARY_DIR}/occa/occa_version_info.h
                     -D WORKING_DIRECTORY=${CMAKE_SOURCE_DIR}
                     -P ${CMAKE_BINARY_DIR}/FillVersion.cmake
)

set(WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
include(${CMAKE_BINARY_DIR}/Version.cmake)
set(OCCA_VERSION "${VERSION}")
set(OCCA_VERSION_MAJOR "${VERSION_MAJOR}")
set(OCCA_VERSION_MINOR "${VERSION_MINOR}")
set(OCCA_VERSION_PATCH "${VERSION_PATCH}")

if(UNIX)
  file(WRITE ${CMAKE_BINARY_DIR}/Dist.cmake
"message(STATUS \"Package Dist running...\")
include(CPackSourceConfig.cmake)
execute_process(COMMAND cpack -G TGZ --config CPackSourceConfig.cmake
  TIMEOUT 3600
  WORKING_DIRECTORY \${CMAKE_BINARY_DIR})
")
  #
  # Add custom target
  #
  add_custom_target(dist
    COMMAND ${CMAKE_COMMAND} .
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/Dist.cmake
    )
  add_dependencies(dist version)
endif(UNIX)



##
## Library
##
file2header(${CMAKE_SOURCE_DIR}/libocca/occaCLdefines.hpp
            ${CMAKE_BINARY_DIR}/occa/occaCLdefinesString.h)
file2header(${CMAKE_SOURCE_DIR}/libocca/occaCPUdefines.hpp
            ${CMAKE_BINARY_DIR}/occa/occaCPUdefinesString.h)
file2header(${CMAKE_SOURCE_DIR}/libocca/occaCUdefines.hpp
            ${CMAKE_BINARY_DIR}/occa/occaCUdefinesString.h)
add_definitions( -DOCCA_STRINGIFY_DEFINES="${OCCA_STRINGIFY_DEFINES}")

##
## Map Fortran SYMBOLS we need
##
FortranCInterface_HEADER(occa/occaf_f2c.h
                         MACRO_NAMESPACE "OCCAF_F2C_"
                         SYMBOL_NAMESPACE "OCCAF_"
                         SYMBOLS
                         OCCAFNEW_C
                         OCCAFSETUP_C
                         OCCAFBUILDKERNEL_C
                         OCCAFSETTHREADARRAY_C
                         OCCAFCREATEBUFFER_NULL_C
                         OCCAFCREATEBUFFER_C
                         OCCAFSETKERNELARG_MEM_C
                         OCCAFSETKERNELARG_INT4_C
                         OCCAFSETKERNELARG_INT8_C
                         OCCAFSETKERNELARG_REAL4_C
                         OCCAFSETKERNELARG_REAL8_C
                         OCCAFSETKERNELARG_CHAR_C
                         OCCAFQUEUEKERNEL_C
                         OCCAFTOHOST_INT4_C
                         OCCAFTOHOST_INT8_C
                         OCCAFTOHOST_REAL4_C
                         OCCAFTOHOST_REAL8_C
                         OCCAFTOHOST_CHAR_C
                         OCCAFDESTROYBUFFER_C
                         OCCAFDESTROYKERNEL_C
                         OCCAFDESTROY_C
                         )

add_library(occa
            libocca/occa.cpp
            liboccac/occac.cpp
            liboccaf/occaf.c
            liboccaf/occaf.f90
            liboccaf/occafTypes.f90
            liboccaf/occafFunctions.f90
            ${CMAKE_BINARY_DIR}/occa/occaCLdefinesString.h
            ${CMAKE_BINARY_DIR}/occa/occaCPUdefinesString.h
            ${CMAKE_BINARY_DIR}/occa/occaCUdefinesString.h)
add_dependencies(occa version)
install(TARGETS occa
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
if(OPENCL_FOUND)
  target_link_libraries(occa ${OPENCL_LIBRARIES})
endif(OPENCL_FOUND)
if(CUDA_FOUND)
  target_link_libraries(occa ${CUDA_CUDA_LIBRARY})
endif(CUDA_FOUND)
target_link_libraries(occa ${CMAKE_DL_LIBS})




##
## Examples
##
add_subdirectory(examples/simple)
add_subdirectory(examples/fd2d)


##
## Print configuration
##
set(OCCA_COMPILE_LIST
  "             OCCA_DEBUG:                   ${OCCA_DEBUG}\\\\n"
  "             OCCA_HAVE_OPENCL:             ${OCCA_HAVE_OPENCL}\\\\n"
  "             OCCA_HAVE_CUDA:               ${OCCA_HAVE_CUDA}\\\\n"
  "             OCCA_HAVE_OPENMP:             ${OCCA_HAVE_CUDA}\\\\n"
  #  "             OCCA_HAVE_TIME_H:             ${OCCA_HAVE_TIME_H}\\\\n"
  #  "             OCCA_HAVE_SYS_TIME_H:         ${OCCA_HAVE_SYS_TIME_H}\\\\n"
  #  "             OCCA_HAVE_MACH_MACH_TIME_H:   ${OCCA_HAVE_MACH_MACH_TIME_H}\\\\n"
  #  "             OCCA_HAVE_GETTIMEOFDAY:       ${OCCA_HAVE_GETTIMEOFDAY}\\\\n"
  #  "             OCCA_HAVE_CLOCK_MONOTONIC:    ${OCCA_HAVE_CLOCK_MONOTONIC}\\\\n"
  #  "             OCCA_HAVE_MACH_ABSOLUTE_TIME: ${OCCA_HAVE_MACH_ABSOLUTE_TIME}\\\\n"
  "             OCCA_USE_OPENCL:              ${OCCA_USE_OPENCL}\\\\n"
  "             OCCA_USE_CUDA:                ${OCCA_USE_CUDA}\\\\n"
  "             OCCA_USE_CPU:                 ${OCCA_USE_CPU}\\\\n"
  "             OCCA_USE_ALL:                 ${OCCA_USE_ALL}\\\\n"
  "             OCCA_CPU_COMPILER:            ${OCCA_CPU_COMPILER}\\\\n"
  "             OCCA_CPU_FLAGS:               ${OCCA_CPU_FLAGS}\\\\n"
  "             OCCA_CU_COMPILER:             ${OCCA_CU_COMPILER}\\\\n"
  "             OCCA_CU_FLAGS:                ${OCCA_CU_FLAGS}\\\\n"
  "             OCCA_CL_FLAGS:                ${OCCA_CL_FLAGS}\\\\n"
  "             CMAKE_CXX_COMPILER:           ${CMAKE_CXX_COMPILER}\\\\n"
  "             CMAKE_CXX_FLAGS:              ${CMAKE_CXX_FLAGS}\\\\n"
  "             CMAKE_CXX_FLAGS_DEBUG:        ${CMAKE_CXX_FLAGS_DEBUG}\\\\n"
  "             CMAKE_CXX_FLAGS_RELEASE:      ${CMAKE_CXX_FLAGS_RELEASE}\\\\n"
  "             CMAKE_C_COMPILER:             ${CMAKE_C_COMPILER}\\\\n"
  "             CMAKE_C_FLAGS:                ${CMAKE_C_FLAGS}\\\\n"
  "             CMAKE_C_FLAGS_DEBUG:          ${CMAKE_C_FLAGS_DEBUG}\\\\n"
  "             CMAKE_C_FLAGS_RELEASE:        ${CMAKE_C_FLAGS_RELEASE}\\\\n"
  "             CMAKE_Fortran_COMPILER:       ${CMAKE_Fortran_COMPILER}\\\\n"
  "             CMAKE_Fortran_FLAGS:          ${CMAKE_Fortran_FLAGS}\\\\n"
  "             CMAKE_Fortran_FLAGS_DEBUG:    ${CMAKE_Fortran_FLAGS_DEBUG}\\\\n"
  "             CMAKE_Fortran_FLAGS_RELEASE:  ${CMAKE_Fortran_FLAGS_RELEASE}\\\\n"
  "             OPENCL_FOUND:                 ${OPENCL_FOUND}\\\\n"
  "             OPENCL_INCLUDE_DIRS:          ${OPENCL_INCLUDE_DIRS}\\\\n"
  "             OPENCL_LIBRARIES:             ${OPENCL_LIBRARIES}\\\\n"
  "             OPENMP_FOUND:                 ${OPENMP_FOUND}\\\\n"
  "             OpenMP_C_FLAGS:               ${OpenMP_C_FLAGS}\\\\n"
  "             OpenMP_CXX_FLAGS:             ${OpenMP_CXX_FLAGS}\\\\n"
  "             CUDA_FOUND:                   ${CUDA_FOUND}\\\\n"
  "             CUDA_VERSION:                 ${CUDA_VERSION}\\\\n"
  "             CUDA_INCLUDE_DIRS:            ${CUDA_INCLUDE_DIRS}\\\\n"
  "             CUDA_CUDA_LIBRARY:            ${CUDA_CUDA_LIBRARY}\\\\n"
  "             CUDA_NVCC_EXECUTABLE:         ${CUDA_NVCC_EXECUTABLE}\\\\n"
  "             CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS:           ${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS}\\\\n"
  "             CMAKE_CXX_COMPILE_OPTIONS_PIC:                   ${CMAKE_CXX_COMPILE_OPTIONS_PIC}\\\\n"
  "             CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS:           ${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS}\\\\n"
  "             CMAKE_CXX_COMPILE_OPTIONS_PIC:                   ${CMAKE_CXX_COMPILE_OPTIONS_PIC}\\\\n"
  "             CMAKE_CXX_COMPILE_OPTIONS_PIE:                   ${CMAKE_CXX_COMPILE_OPTIONS_PIE}\\\\n"
  "             CMAKE_CXX_COMPILE_OPTIONS_DLL:                   ${CMAKE_CXX_COMPILE_OPTIONS_DLL}\\\\n"
  "             CMAKE_SHARED_LIBRARY_CXX_FLAGS:                  ${CMAKE_SHARED_LIBRARY_CXX_FLAGS}\\\\n"
  "             CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS:             ${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}\\\\n"
  "             CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG:           ${CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG}\\\\n"
  "             CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG_SEP:       ${CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG_SEP}\\\\n"
  "             CMAKE_SHARED_LIBRARY_RPATH_LINK_CXX_FLAG:        ${CMAKE_SHARED_LIBRARY_RPATH_LINK_CXX_FLAG}\\\\n"
  "             CMAKE_EXE_EXPORTS_CXX_FLAG:                      ${CMAKE_EXE_EXPORTS_CXX_FLAG}\\\\n"
  "             CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG:            ${CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG}\\\\n"
  "             CMAKE_EXECUTABLE_RUNTIME_CXX_FLAG:               ${CMAKE_EXECUTABLE_RUNTIME_CXX_FLAG}\\\\n"
  "             CMAKE_EXECUTABLE_RUNTIME_CXX_FLAG_SEP:           ${CMAKE_EXECUTABLE_RUNTIME_CXX_FLAG_SEP}\\\\n"
  "             CMAKE_EXECUTABLE_RPATH_LINK_CXX_FLAG:            ${CMAKE_EXECUTABLE_RPATH_LINK_CXX_FLAG}\\\\n"
  "             CMAKE_SHARED_LIBRARY_LINK_CXX_WITH_RUNTIME_PATH: ${CMAKE_SHARED_LIBRARY_LINK_CXX_WITH_RUNTIME_PATH}\\\\n"
  "             CMAKE_INCLUDE_FLAG_CXX:                          ${CMAKE_INCLUDE_FLAG_CXX}\\\\n"
  "             CMAKE_INCLUDE_FLAG_SEP_CXX:                      ${CMAKE_INCLUDE_FLAG_SEP_CXX}\\\\n"
  "")

function(JOIN VALUES GLUE OUTPUT)
  string (REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" _TMP_STR "${VALUES}")
  string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}") #fixes escaping
  set (${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction()

join("${OCCA_COMPILE_LIST}" "" OCCA_COMPILE_INFO)

##
## Write config file
##
## configure_file(${CMAKE_SOURCE_DIR}/occa/occaConfig.h.in
##                ${CMAKE_BINARY_DIR}/occa/occaConfig.h)



##
## Set the CPack options
##
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OCCA")
set(CPACK_PACKAGE_VENDOR "")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${OCCA_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_VERSION_MAJOR ${OCCA_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${OCCA_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${OCCA_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "OCCA-${OCCA_VERSION_FULL}")
set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${CMAKE_SOURCE_DIR};/;${CMAKE_BINARY_DIR}/source;/")
set(CPACK_SOURCE_IGNORE_FILES
"~$"
"\\\\.sw[a-z]$"
"\\\\.DS_Store"
"tags"
"cscope.*"
".*\\\\.mod"
"^${PROJECT_SOURCE_DIR}/\\\\.git.*"
"^${PROJECT_SOURCE_DIR}/build.*"
)

# This should be the last line
include(CPack)
