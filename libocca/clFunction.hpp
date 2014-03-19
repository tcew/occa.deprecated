#ifndef __CLFUNCTION
#define __CLFUNCTION

#include "genFunction.hpp"

class clHelper;

class clFunction : public genFunction {

public:

  cl_kernel  kernel;
  cl_program program;

  char          *name;
  char          *source;
  cl_device_id  *device;
  cl_context    *context;
  cl_command_queue      *queue;

  cl_event event;

  cl_mem cldims;

  int firstTic, firstToc;

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

  clFunction(){
    for(int i=0;i<3;++i){
      local[i] = 1;
      global[i] = 1;
    }

    firstTic = 1;
    firstToc = 1;
  }

  clFunction(cl_context *in_context, cl_device_id *in_device, cl_command_queue *in_queue,
             const char *sourcefilename, const char *functionname, const char *flags){
    buildFromSource(in_context, in_device, in_queue, sourcefilename, functionname, flags);
  }

  clFunction(const clFunction &c){
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

    kernel  = c.kernel;
    program = c.program;

    name    = c.name;
    source  = c.source;
    device  = c.device;
    context = c.context;
    queue   = c.queue;

    event = c.event;

    cldims = c.cldims;

    firstTic = c.firstTic;
    firstToc = c.firstToc;
  }

  clFunction& operator = (const clFunction &c){
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

    kernel  = c.kernel;
    program = c.program;

    name    = c.name;
    source  = c.source;
    device  = c.device;
    context = c.context;
    queue   = c.queue;

    event = c.event;

    cldims = c.cldims;

    firstTic = c.firstTic;
    firstToc = c.firstToc;

    return *this;
  }

#ifndef OCCA_CL_FLAGS
#define OCCA_CL_FLAGS "-cl-single-precision-constant -I. -cl-denorms-are-zero -cl-single-precision-constant -cl-fast-relaxed-math -cl-finite-math-only -cl-mad-enable  -cl-no-signed-zeros"
#endif

  clFunction& buildFromSource(cl_context *in_context, cl_device_id *in_device, cl_command_queue *in_queue,
                              const char *sourcefilename, const char *functionname, const char *flags){
    firstTic = 1;
    firstToc = 1;

    int err;

    for(int i=0;i<3;++i){
      local[i] = 1;
      global[i] = 1;
    }

    context = in_context;
    device  = in_device;
    queue   = in_queue;

    name = strdup(functionname);

    load_program_source(sourcefilename);

    program = clCreateProgramWithSource(*context, 1, (const char **) & source, (size_t*) NULL, &err);

    if (!program){
      printf("Error: Failed to create compute program!\n");
      throw 1;
    }

    // Build the program executable
    char all_flags[BUFSIZ*2];
    if(flags)
      //sprintf(all_flags, "-I. -cl-finite-math-only -cl-no-signed-zeros -cl-strict-aliasing %s", flags);
      sprintf(all_flags, "%s %s", OCCA_CL_FLAGS, flags);
    //      sprintf(all_flags, "-I. %s", flags);
    else
      sprintf(all_flags, OCCA_CL_FLAGS);
    //      sprintf(all_flags, "-I. ");

    err = clBuildProgram(program, 1, device, all_flags, (void (*)(cl_program, void*))  NULL, NULL);

    char *build_log;
    size_t ret_val_size;
    err = clGetProgramBuildInfo(program, *device, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);

    build_log = (char*) malloc(ret_val_size+1);
    err = clGetProgramBuildInfo(program, *device, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, (size_t*) NULL);

    // to be carefully, terminate with \0
    // there's no information in the reference whether the string is 0 terminated or not
    build_log[ret_val_size] = '\0';

    fprintf(stderr, "%s", build_log );

    kernel = clCreateKernel(program, functionname, &err);
    if (! kernel || err != CL_SUCCESS){
      printf("Error: Failed to create compute kernel!\n");
      throw 1;
    }

    cldims = clCreateBuffer(*context, CL_MEM_READ_WRITE, 3*sizeof(int), NULL, &err);

    return *this;
  }

  clFunction& buildFromBinary(cl_context *in_context, cl_device_id *in_device, cl_command_queue *in_queue,
                              const char *sourcefilename, const char *functionname, const char *flags){
    firstTic = 1;
    firstToc = 1;

    int err;

    for(int i=0;i<3;++i){
      local[i] = 1;
      global[i] = 1;
    }

    context = in_context;
    device  = in_device;
    queue   = in_queue;

    name = strdup(functionname);

    //---[ Load Binary ]------
    struct stat statbuf;
    FILE *fh = fopen(sourcefilename, "rb");

    if (fh == 0){
      printf("Failed to open: %s\n", sourcefilename);
      throw 1;
    }

    stat(sourcefilename, &statbuf);
    size_t binarySize = statbuf.st_size;
    char *binary = (char *) malloc(binarySize + 1);
    fread(binary, binarySize, 1, fh);
    binary[binarySize] = '\0';
    //========================

    program = clCreateProgramWithBinary(*context, 1, device, &binarySize, (const unsigned char **) &binary, (cl_int*) NULL, &err);

    if (!program){
      printf("Error: Failed to create compute program!\n");
      throw 1;
    }

    // Build the program executable
    char all_flags[BUFSIZ*2];
    if(flags)
      //sprintf(all_flags, "-I. -cl-finite-math-only -cl-no-signed-zeros -cl-strict-aliasing %s", flags);
      sprintf(all_flags, "%s %s", OCCA_CL_FLAGS, flags);
    //      sprintf(all_flags, "-I. %s", flags);
    else
      sprintf(all_flags, OCCA_CL_FLAGS);
    //      sprintf(all_flags, "-I. ");

    err = clBuildProgram(program, 1, device, all_flags, (void (*)(cl_program, void*))  NULL, NULL);

    char *build_log;
    size_t ret_val_size;
    err = clGetProgramBuildInfo(program, *device, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);

    build_log = (char*) malloc(ret_val_size+1);
    err = clGetProgramBuildInfo(program, *device, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, (size_t*) NULL);

    // to be carefully, terminate with \0
    // there's no information in the reference whether the string is 0 terminated or not
    build_log[ret_val_size] = '\0';

    fprintf(stderr, "%s", build_log );

    kernel = clCreateKernel(program, functionname, &err);
    if (! kernel || err != CL_SUCCESS){
      printf("Error: Failed to create compute kernel!\n");
      throw 1;
    }

    cldims = clCreateBuffer(*context, CL_MEM_READ_WRITE, 3*sizeof(int), NULL, &err);

    free(binary);

    return *this;
  }

  void setThreadArray(size_t *in_global, size_t *in_local, int in_dim){
    dim = in_dim;

    for(int i=0;i<dim;++i){
      global[i] = in_global[i];
      local[i] = in_local[i];

      if(global[i] == 0 || global[i] > MAX_GLOBAL_SIZE){
        printf("Error: global[%d] = %lu is invalid !\n", i, global[i]);
        throw 1;
      }

      if(local[i] == 0 || local[i] > MAX_LOCAL_SIZE){
        printf("Error: local[%d] = %lu, is invalid !\n", i, local[i]);
        throw 1;
      }

      if(global[i]%local[i]){
        printf("Error: global thread array size global[%d] = %lu, is not divisible by local thread array size local[%d] = %lu! \n", i, global[i], i, local[i]);
        throw 1;
      }
    }
  }

  void setThreadDims(int dim_, occaDim groups, occaDim items){
    dim = dim_;

    for(int i = 0; i < dim; ++i){
      global[i] = groups[i]*items[i];
      local[i]  = items[i];

      if(global[i] == 0 || global[i] > MAX_GLOBAL_SIZE){
        printf("Error: global[%d] = %lu is invalid !\n", i, global[i]);
        throw 1;
      }

      if(local[i] == 0 || local[i] > MAX_LOCAL_SIZE){
        printf("Error: local[%d] = %lu, is invalid !\n", i, local[i]);
        throw 1;
      }
    }
  }

  void enqueue(int argc, void* args[], size_t argssz[])
  {
    CL_CHECK(clSetKernelArg(kernel, 0, sizeof(cldims), &cldims));
    for(int i = 0; i < argc; ++i)
    {
      if(argssz[i] > 0)
      {
        CL_CHECK(clSetKernelArg(kernel, i+1, argssz[i], args[i]));
      }
      else
      {
        CL_CHECK(clSetKernelArg(kernel, i+1, sizeof(cl_mem), args[i]));
      }
    }
    run();
  }

  void tic(){

  }


  void toc(){

  }

  double elapsedTime()
  {
    // return elapsed time between two queue events (in seconds)
    // Note: caller must call clFinish on target queue

    clWaitForEvents(1 , &event);

    clFinish(*queue);

    cl_ulong start, end;

    CL_CHECK(clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,   sizeof(cl_ulong), &end,   ulNULL) );
    CL_CHECK(clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, ulNULL) );

    // return elapsed time (seconds)
    return  (double) (1.0e-9 * (double)(end - start));
  }


  void run(){

    tic();

    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, &event));

    toc();

  }

  CL_KERNEL_OPERATORS;

  int preferredWorkgroupMultiple(){
    cl_device_type type;
    clGetDeviceInfo(*device, CL_DEVICE_TYPE, sizeof(type), &type, NULL);

    if(type == CL_DEVICE_TYPE_GPU){
      size_t pref;

      clGetKernelWorkGroupInfo(kernel,
                               *device,
                               CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                               sizeof(size_t),
                               &pref,
                               (unsigned long int*) NULL);
      return pref;
    }

    return 1;
  }

};
#endif
