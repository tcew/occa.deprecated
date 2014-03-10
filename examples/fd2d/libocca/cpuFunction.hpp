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
