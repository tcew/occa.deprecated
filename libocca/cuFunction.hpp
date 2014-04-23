#ifndef __CUFUNCTION
#define __CUFUNCTION

#include <cuda.h>

class occaMemory;

// simple class that acts as an intermediate argument class for clSetKernelArg
// [ limited to: int, float, double, char, cl_mem ]

#include "genFunction.hpp"

class cuHelper;

class cuFunction : public genFunction {

public:

  CUfunction  kernel;
  CUmodule    module;

  char          objectName[BUFSIZ];
  char        *name;
  char        *source;
  CUdevice    *device;
  CUcontext   *context;

  CUevent ev_start, ev_end;

  CUdeviceptr cudims;

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

  cuFunction(){
  }

  cuFunction(CUcontext *in_context, CUdevice *in_device,
             const char *sourcefilename, const char *functionname, const char *flags){
    buildFromSource(in_context, in_device, sourcefilename, functionname, flags);
  }

  cuFunction(const cuFunction &c){
    *this = c;
  }

  cuFunction& operator = (const cuFunction &c){
    dim = c.dim;

    dim = (dim > 3 || dim < 0) ? 1:dim;

    for(int i = 0; i < dim; i++){
      local[i]  = c.local[i];
      global[i] = c.global[i];
    }

    for(int i = dim; i < 3; i++){
      local[i]  = 1;
      global[i] = 1;
    }

    kernel = c.kernel;
    module = c.module;

    name = c.name;
    source = c.source;
    device = c.device;
    context = c.context;

    ev_start = c.ev_start;
    ev_end = c.ev_end;

    cudims = c.cudims;

    return *this;
  }

#ifndef OCCA_CU_COMPILER
#define OCCA_CU_COMPILER "nvcc"
#endif

#ifndef OCCA_CU_FLAGS
#define OCCA_CU_FLAGS  "-ptx -I. -m64 -arch=sm_35 --compiler-options -O3 --use_fast_math "
#endif

  cuFunction& buildFromSource(CUcontext *in_context, CUdevice *in_device,
                              const char *sourcefilename, const char *functionname, const char *flags){
    CUresult err;

    for(int i=0;i<3;++i){
      local[i] = 1;
      global[i] = 1;
    }

    context = in_context;
    device  = in_device;

    char cmd[BUFSIZ];
    int pid = getpid();

    sprintf(objectName, ".occa/%s_%d.ptx", functionname, pid);

    sprintf(cmd, "cp %s .occa/%s_%d.cu ; %s %s %s .occa/%s_%d.cu -o %s",
        sourcefilename, functionname, pid,
        OCCA_CU_COMPILER,
        flags,
        OCCA_CU_FLAGS,
        functionname, pid, objectName);

    system(cmd);

    checkCudaErrors( cuModuleLoad(&module, objectName) );

    // if (err != CUDA_SUCCESS) {
    //   fprintf(stderr, "* Error loading the module %s\n", objectName);
    //   cuCtxDetach(*context);
    //   throw 1;
    // }

    checkCudaErrors( cuModuleGetFunction(&kernel, module, functionname));

    // if (err != CUDA_SUCCESS) {
    //   fprintf(stderr, "* Error getting kernel function %s\n", functionname);
    //   cuCtxDetach(*context);
    //   throw 1;
    // }


    cuEventCreate(&ev_start, CU_EVENT_DEFAULT);
    cuEventCreate(&ev_end, CU_EVENT_DEFAULT);

    checkCudaErrors( cuMemAlloc(&cudims, 3*sizeof(int)) );

    return *this;
  }

  cuFunction& buildFromBinary(CUcontext *in_context, CUdevice *in_device,
                              const char *sourcefilename, const char *functionname, const char *flags){
#warning Have not implemented cuFunction buildFromBinary
    return *this;
  }

  void setThreadArray(size_t *in_global, size_t *in_local, int in_dim){
    dim = in_dim;

    CUdevprop properties;
    checkCudaErrors( cuDeviceGetProperties(&properties, *device) );

    for(int i=0;i<dim;++i){
      global[i] = in_global[i];
      local[i] = in_local[i];

      if(global[i] <= 0 || global[i] >= properties.maxGridSize[i]){
        printf("Error: global[%d] = %lu is invalid !\n", i, global[i]);
        throw 1;
      }

      if(local[i] <= 0 || local[i] > properties.maxThreadsDim[i]){
        printf("Error: local[%d] = %lu, is invalid !\n", i, local[i]);
        throw 1;
      }

      if((global[i]) % (local[i])){
        printf("Error: global thread array size global[%d] = %lu, is not divisible by local thread array size local[%d] = %lu! \n", i, global[i], i, local[i]);
        throw 1;
      }
    }
  }

  void setThreadDims(int dim_, occaDim groups, occaDim items){
    dim = dim_;

    CUdevprop properties;
    checkCudaErrors( cuDeviceGetProperties(&properties, *device) );

    for(int i = 0; i < dim; ++i){
      global[i] = groups[i]*items[i];
      local[i]  = items[i];

      if(global[i] <= 0 || global[i] >= properties.maxGridSize[i]){
        printf("Error: global[%d] = %lu is invalid !\n", i, global[i]);
        throw 1;
      }

      if(local[i] <= 0 || local[i] > properties.maxThreadsDim[i]){
        printf("Error: local[%d] = %lu, is invalid !\n", i, local[i]);
        throw 1;
      }
    }
  }

  void enqueue(int argc, void* args[], size_t argssz[])
  {
    void *local_args[OCCA_MAX_NUM_ARGS+1];

    if(argc > OCCA_MAX_NUM_ARGS){
      printf("Too many arguements: %d", argc);
      throw 1;
    }

    local_args[0] = &cudims;
    for(int i = 0; i < argc; ++i)
      local_args[i+1] = args[i];

    cuEventRecord(ev_start,0);
    cuLaunchKernel(kernel,
                   global[0]/local[0], global[1]/local[1], 1,
                   local[0], local[1], local[2],
                   0, 0, local_args, 0);
    cuEventRecord(ev_end,0);
  }

  void tic(){

  }


  void toc(){

  }

  double elapsedTime()
  {
    cuEventSynchronize(ev_end);

    float elapsed;
    cuEventElapsedTime(&elapsed, ev_start, ev_end);
    return 1e-3*elapsed;

  }

  CU_KERNEL_OPERATORS;

  int preferred_workgroup_multiple(){

    return 1;
  }


};
#endif
