#ifndef __CPUFUNCTION
#define __CPUFUNCTION

#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "genFunction.hpp"

#ifdef        OS_LINUX
#  include <sys/time.h>
#elif defined OS_OSX
#  include <CoreServices/CoreServices.h>
#  include <mach/mach_time.h>
#else
#  error "Not tested on Windows OS yet."
#endif

class occaMemory;
class cpuHelper;

FUNCTION_POINTER_TYPEDEF;

class cpuFunction : public genFunction {

public:

  char          *name;
  char          *source;
  char          objectName[BUFSIZ];
  char          *functionName;
  voidFunction kernel;

  int  dim;
  int dims[6];

  occaTime ev_start, ev_end;

  void load_program_source(const char *filename) {

    struct stat statbuf;
    FILE *fh = fopen(filename, "r");
    if (fh == 0){
      printf("Failed to open: %s\n", filename);
      throw 1;
    }

    stat(filename, &statbuf);
    source = (char *) malloc(statbuf.st_size + 1);
    fread(source, statbuf.st_size, 1, fh);
    source[statbuf.st_size] = '\0';

  }


public:

  cpuFunction(){
  }

  cpuFunction(const string sourcefilename, string functionname, const string flags){
    buildFromSource(sourcefilename, functionname, flags);
  }

  cpuFunction(const cpuFunction &c){
    *this = c;
  }

  cpuFunction& operator = (const cpuFunction &c){
    dim = c.dim;

    dim = (dim > 3 || dim < 0) ? 1:dim;

    for(int i = 0; i < 6; i++)
      dims[i] = c.dims[i];

    for(int i = 0; i < dim; i++){
      local[i]  = c.local[i];
      global[i] = c.global[i];
    }

    for(int i = dim; i < 3; i++){
      local[i]  = 1;
      global[i] = 1;
    }

    name = c.name;
    source = c.source;
    functionName = c.functionName;
    kernel = c.kernel;

    ev_start = c.ev_start;
    ev_end = c.ev_end;

    return *this;
  }

  cpuFunction& buildFromSource(const string sourcefilename, string functionname, const string flags){
    int err;

    char cmd[BUFSIZ];
    sprintf(objectName, ".occa/%s_%d.so", functionname.c_str(), getpid());

#if 0
    sprintf(cmd, "g++ %s -m64 -fopenmp -I. -x c++ -w -fPIC -shared %s -E -ldl",
	    flags.c_str(), sourcefilename.c_str());
    system(cmd);
#endif

#if 1
        sprintf(cmd, "g++ %s -m64 "
	    " -fopenmp -I. -x c++ -w -fPIC -shared %s -o %s  -ldl "
            " -ftree-vectorizer-verbose=1"
	    " -O3 -mtune=native -ftree-vectorize -funroll-loops -fsplit-ivs-in-unroller -ffast-math",
	    flags.c_str(), sourcefilename.c_str(), objectName);
#else
    sprintf(cmd, "icpc %s -m64 -fopenmp -O3 "
	    " -I. -x c++ -w -fPIC -shared %s -o %s  -ldl"
            " -vec-report2"
            " -no-ansi-alias"
            " -funroll-all-loops -ftree-vectorize -fp-model fast=2 -xHost -ftz"
	    " -O3 -mtune=native -ftree-vectorize -fsplit-ivs-in-unroller -ffast-math",
	    flags.c_str(), sourcefilename.c_str(), objectName);
#endif

    cout << cmd << endl;

    int systemReturn = system(cmd);
    if(systemReturn)
      cout << systemReturn;

    functionName = strdup(functionname.c_str());

    void *obj = dlopen(objectName, RTLD_NOW);
    kernel    = (voidFunction) dlsym(obj, functionName);

    return *this;
  }

  cpuFunction& buildFromBinary(const string binaryFilename, string functionname, const string flags){
    functionName = strdup(functionname.c_str());

    void *obj = dlopen(objectName, RTLD_NOW);
    kernel    = (voidFunction) dlsym(obj, functionName);

    return *this;
  }

  void setThreadArray(size_t *in_global, size_t *in_local, int in_dim){
    dim = in_dim;

    dims[0] = in_local[0];
    dims[1] = in_local[1];
    dims[2] = in_local[2];

    dims[3] = in_global[0]/in_local[0];
    dims[4] = in_global[1]/in_local[1];
    dims[5] = 1;
  }

  void setThreadDims(int dim_, occaDim groups, occaDim items){
    dim = dim_;

    dims[0] = items.x;
    dims[1] = items.y;
    dims[2] = items.z;

    dims[3] = groups.x;
    dims[4] = groups.y;
    dims[5] = groups.z;
  }

  CPU_KERNEL_OPERATORS;

  void enqueue(int argc, void* args[], size_t argssz[]){
#define OCCA_ARGS_1                 args[0]
#define OCCA_ARGS_2  OCCA_ARGS_1  , args[1]
#define OCCA_ARGS_3  OCCA_ARGS_2  , args[2]
#define OCCA_ARGS_4  OCCA_ARGS_3  , args[3]
#define OCCA_ARGS_5  OCCA_ARGS_4  , args[4]
#define OCCA_ARGS_6  OCCA_ARGS_5  , args[5]
#define OCCA_ARGS_7  OCCA_ARGS_6  , args[6]
#define OCCA_ARGS_8  OCCA_ARGS_7  , args[7]
#define OCCA_ARGS_9  OCCA_ARGS_8  , args[8]
#define OCCA_ARGS_10 OCCA_ARGS_9  , args[9]
#define OCCA_ARGS_11 OCCA_ARGS_10 , args[10]
#define OCCA_ARGS_12 OCCA_ARGS_11 , args[11]
#define OCCA_ARGS_13 OCCA_ARGS_12 , args[12]
#define OCCA_ARGS_14 OCCA_ARGS_13 , args[13]
#define OCCA_ARGS_15 OCCA_ARGS_14 , args[14]
#define OCCA_ARGS_16 OCCA_ARGS_15 , args[15]
#define OCCA_ARGS_17 OCCA_ARGS_16 , args[16]
#define OCCA_ARGS_18 OCCA_ARGS_17 , args[17]
#define OCCA_ARGS_19 OCCA_ARGS_18 , args[18]
#define OCCA_ARGS_20 OCCA_ARGS_19 , args[19]

#define E(N)                                                        \
    case N:                                                         \
     {                                                              \
        tic();                                                      \
        functionPointer##N tmpKernel = (functionPointer##N) kernel; \
        tmpKernel(dims, OCCA_ARGS_##N);                             \
        toc();                                                      \
      }                                                             \
      break;


    switch(argc){
      FOR_20(E)
      default:
        std::cerr << "Enqueue with the wrong number of arguments" << std::endl;
    }
#undef E

#undef OCCA_ARGS_0
#undef OCCA_ARGS_1
#undef OCCA_ARGS_2
#undef OCCA_ARGS_3
#undef OCCA_ARGS_4
#undef OCCA_ARGS_5
#undef OCCA_ARGS_6
#undef OCCA_ARGS_7
#undef OCCA_ARGS_8
#undef OCCA_ARGS_9
#undef OCCA_ARGS_10
#undef OCCA_ARGS_11
#undef OCCA_ARGS_12
#undef OCCA_ARGS_13
#undef OCCA_ARGS_14
#undef OCCA_ARGS_15
#undef OCCA_ARGS_16
#undef OCCA_ARGS_17
#undef OCCA_ARGS_18
#undef OCCA_ARGS_19
#undef OCCA_ARGS_20
  }

  void tic(){
#ifdef        OS_LINUX
    clock_gettime(CLOCK_MONOTONIC, &ev_start);
#elif defined OS_OSX
    ev_start = mach_absolute_time();
#elif defined OS_WINDOWS
#  error "Not tested on Windows OS yet."
#endif
  }

  void toc(){
#ifdef        OS_LINUX
    clock_gettime(CLOCK_MONOTONIC, &ev_end);
#elif defined OS_OSX
    ev_end = mach_absolute_time();
#elif defined OS_WINDOWS
#  error "Not tested on Windows OS yet."
#endif
  }

  double elapsedTime(){
#ifdef        OS_LINUX
    return (double) (ev_end.tv_sec  - ev_start.tv_sec)
      +    (double) (ev_end.tv_nsec - ev_start.tv_nsec)*1.0e-9;
#elif defined OS_OSX
    const uint64_t timeTaken = (ev_end - ev_start);
    const Nanoseconds nTime  = AbsoluteToNanoseconds(*(AbsoluteTime *) &timeTaken);
    return ((double) 1e-9) * ((double) ( *((uint64_t*) &nTime) ));
#elif defined OS_WINDOWS
#  error "Not tested on Windows OS yet."
#endif
  }

};
#endif
