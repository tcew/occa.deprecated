#ifndef __CLFUNCTION
#define __CLFUNCTION

// simple class that acts as an intermediate argument class for clSetKernelArg
// [ limited to: int, float, double, char, cl_mem ]
class dummy_arg {

public:

  void *pt;
  int sz;

  dummy_arg(){
    pt = NULL;
    sz = 0;
  }

  dummy_arg(int &i){
    pt = &i;
    sz = sizeof(int);
  }

  dummy_arg(float &f){
    pt = &f;
    sz = sizeof(float);
  }

  dummy_arg(double &d){
    pt = &d;
    sz = sizeof(double);
  }

  dummy_arg(char &c){
    pt = &c;
    sz = sizeof(char);
  }


  dummy_arg(cl_mem &cmem){
    pt = &cmem;
    sz = sizeof(cl_mem);
  }

};


class clHelper;

class clKernel {

public:

  cl_kernel  kernel;
  cl_program program;

  char          *name;
  char          *source;
  size_t        source_size;
  char          *binary;
  size_t        binary_size;
  cl_device_id  *device;
  cl_context    *context;
  cl_command_queue      *queue;

  int  dim;
  size_t local[3];
  size_t global[3];

  void load_program_source(const char *filename) {

    struct stat statbuf;
    FILE *fh = fopen(filename, "r");
    if (fh == 0){
      printf("Failed to open: %s\n", filename);
      throw 1;
    }

    stat(filename, &statbuf);
    source_size = statbuf.st_size;
    source = (char *) malloc(source_size + 1);
    fread(source, source_size, 1, fh);
    source[source_size] = '\0';

  }

  void load_program_binary(const char *filename) {

    struct stat statbuf;
    FILE *fh = fopen(filename, "rb");
    if (fh == 0){
      printf("Failed to open: %s\n", filename);
      throw 1;
    }

    stat(filename, &statbuf);
    binary_size = statbuf.st_size;
    binary = (char *) malloc(binary_size + 1);
    fread(binary, binary_size, 1, fh);
    binary[binary_size] = '\0';

  }

  void store_program_binary(const char *filename) {

    FILE *fh = fopen(filename, "wb");
    if (fh == 0){
      printf("Failed to open: %s\n", filename);
      throw 1;
    }

    fwrite(binary, binary_size, 1, fh);

  }


public:

  clKernel(){
  }

  clKernel(cl_context *in_context, cl_device_id *in_device, cl_command_queue *in_queue,
	     const char *sourcefilename, const char *functionname, const char *flags){

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

    program = clCreateProgramWithSource(*context, 1, (const char **) &source, &source_size, &err);

    if (!program){
      printf("Error: Failed to create compute program!\n");
      throw 1;
    }

    // Build the program executable
    char all_flags[BUFSIZ*2];
    if(flags)
      //sprintf(all_flags, "-I. -cl-finite-math-only -cl-no-signed-zeros -cl-strict-aliasing %s", flags);
      sprintf(all_flags, "-I. -cl-finite-math-only -cl-mad-enable  -cl-no-signed-zeros %s", flags);
    else
      sprintf(all_flags, "-I. -cl-finite-math-only -cl-mad-enable  -cl-no-signed-zeros ");
    printf("all_flags = %s\n", all_flags);

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
  }

  clKernel(cl_context *in_context, cl_device_id *in_device, cl_command_queue *in_queue,
	     const char *binaryfilename, const char *functionname){

    int err;

    for(int i=0;i<3;++i){
      local[i] = 1;
      global[i] = 1;
    }

    context = in_context;
    device  = in_device;
    queue   = in_queue;

    name = strdup(functionname);

    load_program_binary(binaryfilename);

    program = clCreateProgramWithBinary(*context, 1, device, &binary_size, (const unsigned char **) &binary, (cl_int *) NULL, &err);

    if (!program){
      printf("Error: Failed to create compute program!\n");
      throw 1;
    }

    err = clBuildProgram(program, 1, device, (const char *) NULL, (void (*)(cl_program, void*))  NULL, NULL);

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
  }

  void setThreadArray(size_t *in_global, size_t *in_local, int in_dim){
    dim = in_dim;

    for(int i=0;i<dim;++i){
      global[i] = in_global[i];
      local[i] = in_local[i];
    }
  }

  // overload operator() with one argument to set one CL argument and queue kernel
  void operator()( dummy_arg d0 ){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));

    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }


  // overload operator() with two arguments to set two CL arguments and queue kernel
  void operator()( dummy_arg d0, dummy_arg d1 ){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));

    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }


  // overload operator() with three arguments to set three CL arguments and queue kernel
  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));

    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }

  // overload operator() with four arguments to set four CL arguments and queue kernel
  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2, dummy_arg d3){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));
    CL_CHECK( clSetKernelArg(kernel, 3, d3.clsz, d3.clpt));

    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }

  // overload operator() with five arguments to set five CL arguments and queue kernel
  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2, dummy_arg d3, dummy_arg d4){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));
    CL_CHECK( clSetKernelArg(kernel, 3, d3.clsz, d3.clpt));
    CL_CHECK( clSetKernelArg(kernel, 4, d4.clsz, d4.clpt));

    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );


  }

  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2, dummy_arg d3, dummy_arg d4, dummy_arg d5){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));
    CL_CHECK( clSetKernelArg(kernel, 3, d3.clsz, d3.clpt));
    CL_CHECK( clSetKernelArg(kernel, 4, d4.clsz, d4.clpt));
    CL_CHECK( clSetKernelArg(kernel, 5, d5.clsz, d5.clpt));

    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );


  }


  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2, dummy_arg d3, dummy_arg d4, dummy_arg d5, dummy_arg d6){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));
    CL_CHECK( clSetKernelArg(kernel, 3, d3.clsz, d3.clpt));
    CL_CHECK( clSetKernelArg(kernel, 4, d4.clsz, d4.clpt));
    CL_CHECK( clSetKernelArg(kernel, 5, d5.clsz, d5.clpt));
    CL_CHECK( clSetKernelArg(kernel, 6, d6.clsz, d6.clpt));
    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }


  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2, dummy_arg d3, dummy_arg d4, dummy_arg d5, dummy_arg d6, dummy_arg d7){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));
    CL_CHECK( clSetKernelArg(kernel, 3, d3.clsz, d3.clpt));
    CL_CHECK( clSetKernelArg(kernel, 4, d4.clsz, d4.clpt));
    CL_CHECK( clSetKernelArg(kernel, 5, d5.clsz, d5.clpt));
    CL_CHECK( clSetKernelArg(kernel, 6, d6.clsz, d6.clpt));
    CL_CHECK( clSetKernelArg(kernel, 7, d7.clsz, d7.clpt));
    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }

  // 9 args
  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2, dummy_arg d3, dummy_arg d4, dummy_arg d5, dummy_arg d6, dummy_arg d7, dummy_arg d8){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));
    CL_CHECK( clSetKernelArg(kernel, 3, d3.clsz, d3.clpt));
    CL_CHECK( clSetKernelArg(kernel, 4, d4.clsz, d4.clpt));
    CL_CHECK( clSetKernelArg(kernel, 5, d5.clsz, d5.clpt));
    CL_CHECK( clSetKernelArg(kernel, 6, d6.clsz, d6.clpt));
    CL_CHECK( clSetKernelArg(kernel, 7, d7.clsz, d7.clpt));
    CL_CHECK( clSetKernelArg(kernel, 8, d8.clsz, d8.clpt));
    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }

  // 10 args
  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2, dummy_arg d3,
		   dummy_arg d4, dummy_arg d5, dummy_arg d6, dummy_arg d7,
		   dummy_arg d8, dummy_arg d9){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));
    CL_CHECK( clSetKernelArg(kernel, 3, d3.clsz, d3.clpt));
    CL_CHECK( clSetKernelArg(kernel, 4, d4.clsz, d4.clpt));
    CL_CHECK( clSetKernelArg(kernel, 5, d5.clsz, d5.clpt));
    CL_CHECK( clSetKernelArg(kernel, 6, d6.clsz, d6.clpt));
    CL_CHECK( clSetKernelArg(kernel, 7, d7.clsz, d7.clpt));
    CL_CHECK( clSetKernelArg(kernel, 8, d8.clsz, d8.clpt));
    CL_CHECK( clSetKernelArg(kernel, 9, d9.clsz, d9.clpt));
    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }

  // 11 args
  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2, dummy_arg d3,
		   dummy_arg d4, dummy_arg d5, dummy_arg d6, dummy_arg d7,
		   dummy_arg d8, dummy_arg d9, dummy_arg d10){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));
    CL_CHECK( clSetKernelArg(kernel, 3, d3.clsz, d3.clpt));
    CL_CHECK( clSetKernelArg(kernel, 4, d4.clsz, d4.clpt));
    CL_CHECK( clSetKernelArg(kernel, 5, d5.clsz, d5.clpt));
    CL_CHECK( clSetKernelArg(kernel, 6, d6.clsz, d6.clpt));
    CL_CHECK( clSetKernelArg(kernel, 7, d7.clsz, d7.clpt));
    CL_CHECK( clSetKernelArg(kernel, 8, d8.clsz, d8.clpt));
    CL_CHECK( clSetKernelArg(kernel, 9, d9.clsz, d9.clpt));
    CL_CHECK( clSetKernelArg(kernel, 10, d10.clsz, d10.clpt));
    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }

  // 12 args
  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2, dummy_arg d3,
		   dummy_arg d4, dummy_arg d5, dummy_arg d6, dummy_arg d7,
		   dummy_arg d8, dummy_arg d9, dummy_arg d10, dummy_arg d11){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));
    CL_CHECK( clSetKernelArg(kernel, 3, d3.clsz, d3.clpt));
    CL_CHECK( clSetKernelArg(kernel, 4, d4.clsz, d4.clpt));
    CL_CHECK( clSetKernelArg(kernel, 5, d5.clsz, d5.clpt));
    CL_CHECK( clSetKernelArg(kernel, 6, d6.clsz, d6.clpt));
    CL_CHECK( clSetKernelArg(kernel, 7, d7.clsz, d7.clpt));
    CL_CHECK( clSetKernelArg(kernel, 8, d8.clsz, d8.clpt));
    CL_CHECK( clSetKernelArg(kernel, 9, d9.clsz, d9.clpt));
    CL_CHECK( clSetKernelArg(kernel, 10, d10.clsz, d10.clpt));
    CL_CHECK( clSetKernelArg(kernel, 11, d11.clsz, d11.clpt));
    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }


  // 13 args
  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2, dummy_arg d3,
		   dummy_arg d4, dummy_arg d5, dummy_arg d6, dummy_arg d7,
		   dummy_arg d8, dummy_arg d9, dummy_arg d10, dummy_arg d11,
		   dummy_arg d12){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));
    CL_CHECK( clSetKernelArg(kernel, 3, d3.clsz, d3.clpt));
    CL_CHECK( clSetKernelArg(kernel, 4, d4.clsz, d4.clpt));
    CL_CHECK( clSetKernelArg(kernel, 5, d5.clsz, d5.clpt));
    CL_CHECK( clSetKernelArg(kernel, 6, d6.clsz, d6.clpt));
    CL_CHECK( clSetKernelArg(kernel, 7, d7.clsz, d7.clpt));
    CL_CHECK( clSetKernelArg(kernel, 8, d8.clsz, d8.clpt));
    CL_CHECK( clSetKernelArg(kernel, 9, d9.clsz, d9.clpt));
    CL_CHECK( clSetKernelArg(kernel, 10, d10.clsz, d10.clpt));
    CL_CHECK( clSetKernelArg(kernel, 11, d11.clsz, d11.clpt));
    CL_CHECK( clSetKernelArg(kernel, 12, d12.clsz, d12.clpt));
    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }

  // 14 args
  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2, dummy_arg d3,
		   dummy_arg d4, dummy_arg d5, dummy_arg d6, dummy_arg d7,
		   dummy_arg d8, dummy_arg d9, dummy_arg d10, dummy_arg d11,
		   dummy_arg d12,dummy_arg d13){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));
    CL_CHECK( clSetKernelArg(kernel, 3, d3.clsz, d3.clpt));
    CL_CHECK( clSetKernelArg(kernel, 4, d4.clsz, d4.clpt));
    CL_CHECK( clSetKernelArg(kernel, 5, d5.clsz, d5.clpt));
    CL_CHECK( clSetKernelArg(kernel, 6, d6.clsz, d6.clpt));
    CL_CHECK( clSetKernelArg(kernel, 7, d7.clsz, d7.clpt));
    CL_CHECK( clSetKernelArg(kernel, 8, d8.clsz, d8.clpt));
    CL_CHECK( clSetKernelArg(kernel, 9, d9.clsz, d9.clpt));
    CL_CHECK( clSetKernelArg(kernel, 10, d10.clsz, d10.clpt));
    CL_CHECK( clSetKernelArg(kernel, 11, d11.clsz, d11.clpt));
    CL_CHECK( clSetKernelArg(kernel, 12, d12.clsz, d12.clpt));
    CL_CHECK( clSetKernelArg(kernel, 13, d13.clsz, d13.clpt));
    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }

  // 15 args
  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2, dummy_arg d3,
		   dummy_arg d4, dummy_arg d5, dummy_arg d6, dummy_arg d7,
		   dummy_arg d8, dummy_arg d9, dummy_arg d10, dummy_arg d11,
		   dummy_arg d12,dummy_arg d13,dummy_arg d14){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));
    CL_CHECK( clSetKernelArg(kernel, 3, d3.clsz, d3.clpt));
    CL_CHECK( clSetKernelArg(kernel, 4, d4.clsz, d4.clpt));
    CL_CHECK( clSetKernelArg(kernel, 5, d5.clsz, d5.clpt));
    CL_CHECK( clSetKernelArg(kernel, 6, d6.clsz, d6.clpt));
    CL_CHECK( clSetKernelArg(kernel, 7, d7.clsz, d7.clpt));
    CL_CHECK( clSetKernelArg(kernel, 8, d8.clsz, d8.clpt));
    CL_CHECK( clSetKernelArg(kernel, 9, d9.clsz, d9.clpt));
    CL_CHECK( clSetKernelArg(kernel, 10, d10.clsz, d10.clpt));
    CL_CHECK( clSetKernelArg(kernel, 11, d11.clsz, d11.clpt));
    CL_CHECK( clSetKernelArg(kernel, 12, d12.clsz, d12.clpt));
    CL_CHECK( clSetKernelArg(kernel, 13, d13.clsz, d13.clpt));
    CL_CHECK( clSetKernelArg(kernel, 14, d14.clsz, d14.clpt));
    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }


  // 16 args
  void operator()( dummy_arg d0, dummy_arg d1, dummy_arg d2, dummy_arg d3,
		   dummy_arg d4, dummy_arg d5, dummy_arg d6, dummy_arg d7,
		   dummy_arg d8, dummy_arg d9, dummy_arg d10, dummy_arg d11,
		   dummy_arg d12,dummy_arg d13,dummy_arg d14, dummy_arg d15){

    CL_CHECK( clSetKernelArg(kernel, 0, d0.clsz, d0.clpt));
    CL_CHECK( clSetKernelArg(kernel, 1, d1.clsz, d1.clpt));
    CL_CHECK( clSetKernelArg(kernel, 2, d2.clsz, d2.clpt));
    CL_CHECK( clSetKernelArg(kernel, 3, d3.clsz, d3.clpt));
    CL_CHECK( clSetKernelArg(kernel, 4, d4.clsz, d4.clpt));
    CL_CHECK( clSetKernelArg(kernel, 5, d5.clsz, d5.clpt));
    CL_CHECK( clSetKernelArg(kernel, 6, d6.clsz, d6.clpt));
    CL_CHECK( clSetKernelArg(kernel, 7, d7.clsz, d7.clpt));
    CL_CHECK( clSetKernelArg(kernel, 8, d8.clsz, d8.clpt));
    CL_CHECK( clSetKernelArg(kernel, 9, d9.clsz, d9.clpt));
    CL_CHECK( clSetKernelArg(kernel, 10, d10.clsz, d10.clpt));
    CL_CHECK( clSetKernelArg(kernel, 11, d11.clsz, d11.clpt));
    CL_CHECK( clSetKernelArg(kernel, 12, d12.clsz, d12.clpt));
    CL_CHECK( clSetKernelArg(kernel, 13, d13.clsz, d13.clpt));
    CL_CHECK( clSetKernelArg(kernel, 14, d14.clsz, d14.clpt));
    CL_CHECK( clSetKernelArg(kernel, 15, d15.clsz, d15.clpt));
    CL_CHECK( clEnqueueNDRangeKernel(*queue, kernel, dim, 0, global, local, 0, (cl_event*)NULL, (cl_event*)NULL) );

  }



  int preferred_workgroup_multiple(){
    size_t pref;

    clGetKernelWorkGroupInfo(kernel,
			     *device,
			     CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
			     sizeof(size_t),
			     &pref,
			     (unsigned long int*) NULL);
    return pref;
  }


};
#endif
