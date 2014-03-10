#ifndef __CLHELPER
#define __CLHELPER 1

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
using namespace std;

#define ulNULL ((unsigned long int * ) NULL)

using std::string;

#define CL_CHECK(_expr)							\
  do {									\
    cl_int _err = _expr;						\
    if (_err == CL_SUCCESS)						\
      break;								\
    fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err); \
    abort();								\
  } while (0)


#define CL_CHECK_ERR(_expr)						\
  ({									\
    cl_int _err = CL_SUCCESS;                                           \
    __typeof__(_expr) _ret = _expr;                                     \
    if (_err != CL_SUCCESS) {						\
      fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err); \
      abort();								\
    }									\
    _ret;								\
  })


void pfn_notify(const char *errinfo, const void *private_info, size_t cb, void *user_data);

void cl_build_kernel(cl_context context, cl_device_id device,
		     string filename, string flags, cl_kernel *kernel);

#include "clFunction.hpp"

class clHelper {

private:

  /* set up CL */
  cl_int            err;
  cl_platform_id    platforms[100];
  cl_uint           platforms_n;
  cl_device_id      devices[100];
  cl_uint           devices_n ;

  cl_context        context;
  cl_command_queue  queue;
  cl_device_id      device;

  long int memUsage;

  int preferredWGM;

public:

  clHelper(int plat, int dev){

    // get list of platform IDs (platform == implementation of OpenCL)
    CL_CHECK( clGetPlatformIDs(100, platforms, &platforms_n));

    if( plat > platforms_n) {
	cout << "ERROR: platform " <<  plat << " unavailable " << endl;
	throw 1;
      }

    // find all available device IDs on chosen platform (could restrict to CPU or GPU)
    cl_uint dtype = CL_DEVICE_TYPE_ALL;
    CL_CHECK( clGetDeviceIDs( platforms[plat], dtype, 100, devices, &devices_n));

    cout << "devices_n = " << devices_n << endl;

    // choose user specified device
    device = devices[dev];

    // create CL context on chosen device

    context = makeContext(device);

    // create command queue
    queue   = CL_CHECK_ERR( clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &_err));

    memUsage = 0;

    {
      stringstream kFileName;

#if OCCA_MPI_ENABLED
      int mpiRank, mpiProcs;
      MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
      MPI_Comm_size(MPI_COMM_WORLD, &mpiProcs);

      // Don't overwrite other rank files
      if(mpiProcs > 1)
        kFileName << ".occa/" << mpiRank << "_rank/tmpWorkgroupMultipleCheckerKernel.cl";
      else
        kFileName << ".occa/tmpWorkgroupMultipleCheckerKernel.cl";
#else
      kFileName << ".occa/tmpWorkgroupMultipleCheckerKernel.cl";
#endif

      string tmpKernelFileName = kFileName.str();
      string tmpKernelName     = "tmpWorkgroupMultipleCheckerKernel";

      ofstream tmpKernelContents(tmpKernelFileName.c_str());
      tmpKernelContents << "__kernel void " << tmpKernelName << "(const int x){}";
      tmpKernelContents.close();

      clFunction tmpKernel = buildKernel(tmpKernelFileName, tmpKernelName);

      preferredWGM = tmpKernel.preferredWorkgroupMultiple();
    }
  }

  cl_context makeContext(cl_device_id device){
    return CL_CHECK_ERR( clCreateContext((cl_context_properties *)NULL, 1, &device, &pfn_notify, (void*)NULL, &_err));
  }

  cl_context makeGLContext(cl_device_id device, cl_platform_id platform){
#if RIDG_INTEROP
#  ifdef __MAC_OS
    cl_context_properties props[] = {
      CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
      (cl_context_properties) CGLGetShareGroup( CGLGetCurrentContext() ),
      0
    };
#  else
    cl_context_properties props[] = {
      CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
      0
    };
#  endif

    return CL_CHECK_ERR( clCreateContext(props, 0, 0, ulNULL, ulNULL, &_err) );
#else
    return makeContext(device);
#endif
  }


  cl_device_id *getDevice(){
    return &device;
  }

  cl_context *getContext(){
    return &context;
  }

  cl_command_queue *getQueue(){
    return &queue;
  }

  clFunction buildKernel(const string sourcefilename, const string functionname, string defines = "", string flags = ""){
    if(flags == "")
      flags = "-cl-finite-math-only -cl-mad-enable -cl-fast-relaxed-math -cl-no-signed-zeros ";

    if(defines != ""){
      int start = 0;

      for(int i = 0; i < sourcefilename.length(); i++)
        if(sourcefilename[i] == '/')
          start = (i+1);

      int length = sourcefilename.length() - start;
      string source = sourcefilename.substr(start, length);

      char defname[BUFSIZ];

#if OCCA_MPI_ENALBED
      int mpiRank, mpiProcs;
      MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
      MPI_Comm_size(MPI_COMM_WORLD, &mpiProcs);

      if(mpiProcs > 1)
        sprintf(defname, ".occa/%d_rank/%s", mpiRank, source.c_str());
      else
        sprintf(defname, ".occa/%s", source.c_str());
#else
      sprintf(defname, ".occa/%s", source.c_str());
#endif

      ofstream defs(defname);

      defs << defines << "\n";

      ifstream sourceContents(sourcefilename.c_str());

      defs << string(std::istreambuf_iterator<char>(sourceContents), std::istreambuf_iterator<char>());

      defs.close();
      sourceContents.close();

      cout << "building: " << functionname << " from " <<  defname << endl;
      return clFunction(&context, &device, &queue, defname, functionname.c_str(), flags.c_str());
    }
    else{
      cout << "building: " << functionname << " from " <<  sourcefilename << endl;
      return clFunction(&context, &device, &queue, sourcefilename.c_str(), functionname.c_str(), flags.c_str());
    }
  }

  clFunction loadKernelFromBinary(const string sourceFilename, const string functionName){
#warning Needs to be implemented
    return clFunction(&context, &device, &queue, sourceFilename.c_str(), functionName.c_str(), "");
  }

  cl_mem createBuffer(size_t sz, void *data){

    cl_int _err;

    memUsage += sz;

    if(data == NULL)
      return CL_CHECK_ERR ( clCreateBuffer(context, CL_MEM_READ_WRITE, sz, NULL, &_err) );

    return CL_CHECK_ERR ( clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sz, data, &_err) );
  }

#if 0
  cl_mem createImage(int rows, int columns, void *data){

    //#ifdef CL_VERSION_1_2
#if 0
    /// WILL BREAK FOR DOUBLE

    cl_image_format format;
    format.image_channel_order = CL_RGBA;
    format.image_channel_data_type = CL_FLOAT;

    cl_image_desc description;

    description.image_type        = CL_MEM_OBJECT_IMAGE2D;
    description.image_width       = columns;
    description.image_height      = rows;
    description.image_array_size  = 1;
    description.image_depth       = 1;
    description.image_row_pitch   = columns*4;  //rgba mode, that is 400 bytes
    description.image_slice_pitch = rows*columns*4;
    description.num_mip_levels    = 0;
    description.num_samples       = 0;
    description.buffer            = ulNULL;

    format.image_channel_data_type = CL_FLOAT;
    format.image_channel_order = CL_RGBA;

    cl_int _err;

    memUsage += rows*columns*sizeof(data); /// hmmm

    return CL_CHECK_ERR (clCreateImage(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				       &format, &description, data, &_err) );
#else

    cl_image_format format;
    format.image_channel_data_type = CL_FLOAT;
    format.image_channel_order = CL_RGBA;

    cl_int _err;

    memUsage += rows*columns*sizeof(data);

    return CL_CHECK_ERR (clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
					 &format, columns, rows, 0, data, &_err) );


#endif
  }
#endif

  long int reportMemoryUsage(){

    return memUsage;

  }


  void finish(){
    clFinish(queue);
  }


  void flush(){
    clFlush(queue);
  }



  void destroyBuffer(cl_mem a){

    clFinish(queue);

    cl_int _err;
    CL_CHECK_ERR ( clReleaseMemObject(a) );

  }


  void toHost(size_t sz, void *dest, cl_mem &source){

    clFinish(queue);

    // blocking read to host
    CL_CHECK( clEnqueueReadBuffer(queue, source, CL_TRUE, 0, sz, dest, 0, 0, 0));
  }


  void toHost(size_t off, size_t sz, void *dest, cl_mem &source){
    clFinish(queue);

    // blocking read to host
    CL_CHECK( clEnqueueReadBuffer(queue, source, CL_TRUE, off, sz, dest, 0, 0, 0));
  }

  void toDevice(size_t sz, cl_mem dest, void *source){
    // blocking copy to device
    CL_CHECK( clEnqueueWriteBuffer(queue,   dest, CL_TRUE, 0, sz, source, 0, 0, 0));
  }

  void deviceCopy(cl_mem &dest, cl_mem &src,
                  size_t bytes,
                  size_t srcOffset  = 0,
                  size_t destOffset = 0){
    CL_CHECK( clEnqueueCopyBuffer(queue,
                                  src, dest,
                                  srcOffset, destOffset, bytes,
                                  0, NULL, NULL) );
  }

  void queueMarker(cl_event &ev){
#ifdef CL_VERSION_1_2
     clEnqueueMarkerWithWaitList(queue, 0, NULL, &ev);
#else
    clEnqueueMarker            (queue,          &ev);
#endif

    clFlush(queue);
  }


  double elapsedTime(cl_event& ev_start, cl_event& ev_end){
    // return elapsed time between two queue events (in seconds)
    // Note: caller must call clFinish on target queue

    clFinish(queue);

    cl_ulong start, end;

    CL_CHECK ( clGetEventProfilingInfo(ev_end,   CL_PROFILING_COMMAND_END,   sizeof(cl_ulong), &end,   ulNULL) );
    CL_CHECK ( clGetEventProfilingInfo(ev_start, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, ulNULL) );

    // decrement event counts
    clReleaseEvent(ev_end);
    clReleaseEvent(ev_start);

    // return elapsed time (seconds)
    return  (double) (1.0e-9 * (double)(end - start));
  }

  void listDevices(){
    int i;
    cl_platform_id platforms[100];
    cl_uint       platforms_n = 0;
    int plat;

    // Get list of platform IDs (platform == implementation of OpenCL)
    CL_CHECK(clGetPlatformIDs(100, platforms, &platforms_n));

    // Print out information strings for each found platform
    printf("====================================\n");
    printf("=== %d OpenCL platform(s) found: ===\n", platforms_n);
    printf("====================================\n\n");

    // Print out information for each platform
    for (plat=0; plat<platforms_n; ++plat){
      cl_device_id devices[100];
      cl_uint devices_n = 0;
      char buffer[10240];

      printf("-----------------------------------------------\nPLATFORM: %d\n", plat);
      CL_CHECK(clGetPlatformInfo(platforms[plat], CL_PLATFORM_PROFILE, 10240, buffer, NULL));
      printf("PROFILE = %s\n", buffer);
      CL_CHECK(clGetPlatformInfo(platforms[plat], CL_PLATFORM_VERSION, 10240, buffer, NULL));
      printf("VERSION = %s\n", buffer);
      CL_CHECK(clGetPlatformInfo(platforms[plat], CL_PLATFORM_NAME, 10240, buffer, NULL));
      printf("NAME = %s\n", buffer);
      CL_CHECK(clGetPlatformInfo(platforms[plat], CL_PLATFORM_VENDOR, 10240, buffer, NULL));
      printf("VENDOR = %s\n", buffer);
      CL_CHECK(clGetPlatformInfo(platforms[plat], CL_PLATFORM_EXTENSIONS, 10240, buffer, NULL));
      printf("EXTENSIONS = %s\n", buffer);

      // Take any device type (could specify CPU or GPU instead of ALL)
      cl_uint dtype = CL_DEVICE_TYPE_ALL;
      CL_CHECK(clGetDeviceIDs(platforms[plat], dtype, 100, devices, &devices_n));

      // Print out information for each device on this platform
      for (i=0; i<devices_n; i++){
	char buffer[10240];
	cl_uint buf_uint;
	cl_ulong buf_ulong;

	printf("***********************************************\nPLATFORM: %d DEVICE: %d\n", plat, i);
	CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(buffer), buffer, NULL));
	printf("DEVICE_NAME = %s\n", buffer);
	CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL));
	printf("DEVICE_VENDOR = %s\n", buffer);
	CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, sizeof(buffer), buffer, NULL));
	printf("DEVICE_VERSION = %s\n", buffer);
	CL_CHECK(clGetDeviceInfo(devices[i], CL_DRIVER_VERSION, sizeof(buffer), buffer, NULL));
	printf("DRIVER_VERSION = %s\n", buffer);
	CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(buf_uint), &buf_uint, NULL));
	printf("DEVICE_MAX_COMPUTE_UNITS = %u\n", (unsigned int)buf_uint);
	CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(buf_uint), &buf_uint, NULL));
	printf("DEVICE_MAX_CLOCK_FREQUENCY = %u\n", (unsigned int)buf_uint);
	CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(buf_ulong), &buf_ulong, NULL));
	printf("DEVICE_GLOBAL_MEM_SIZE = %llu\n", (unsigned long long)buf_ulong);
	CL_CHECK(clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(buf_ulong), &buf_ulong, NULL));
	printf("DEVICE_MAX_WORK_GROUP_SIZE = %llu\n", (unsigned long long)buf_ulong);

	// CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS
	cl_uint workitem_dims;
	clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(workitem_dims), &workitem_dims, NULL);
	printf("CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:\t%u\n", (unsigned int) workitem_dims);

	size_t workitem_size[3];
	clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(workitem_size), &workitem_size, NULL);
	printf("CL_DEVICE_MAX_WORK_ITEM_SIZES:\t%d / %d / %d \n",
	       (int)workitem_size[0], (int)workitem_size[1], (int)workitem_size[2]);
      }
      printf("-----------------------------------------------\n\n");
    }
  }

  int preferredWorkgroupMultiple(){
    return 1;
    // return preferredWGM;
  }
};

#endif
