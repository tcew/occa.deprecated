#ifndef __CUHELPER
#define __CUHELPER

// derived from https://gist.github.com/tautologico/2879581


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
using namespace std;

#define ulNULL ((unsigned long int * ) NULL)

#include <cuda.h>
#include <builtin_types.h>

using std::string;

static const char * cuHelperErrorToString(int err)
{
  switch(err)
  {
    case CUDA_SUCCESS:
      return "CUDA_SUCCESS";
    case CUDA_ERROR_INVALID_VALUE:
      return "CUDA_ERROR_INVALID_VALUE";
    case CUDA_ERROR_OUT_OF_MEMORY:
      return "CUDA_ERROR_OUT_OF_MEMORY";
    case CUDA_ERROR_NOT_INITIALIZED:
      return "CUDA_ERROR_NOT_INITIALIZED";
    case CUDA_ERROR_DEINITIALIZED:
      return "CUDA_ERROR_DEINITIALIZED";
    case CUDA_ERROR_NO_DEVICE:
      return "CUDA_ERROR_NO_DEVICE";
    case CUDA_ERROR_INVALID_DEVICE:
      return "CUDA_ERROR_INVALID_DEVICE";
    case CUDA_ERROR_INVALID_IMAGE:
      return "CUDA_ERROR_INVALID_IMAGE";
    case CUDA_ERROR_INVALID_CONTEXT:
      return "CUDA_ERROR_INVALID_CONTEXT";
    case CUDA_ERROR_CONTEXT_ALREADY_CURRENT:
      return "CUDA_ERROR_CONTEXT_ALREADY_CURRENT";
    case CUDA_ERROR_MAP_FAILED:
      return "CUDA_ERROR_MAP_FAILED";
    case CUDA_ERROR_UNMAP_FAILED:
      return "CUDA_ERROR_UNMAP_FAILED";
    case CUDA_ERROR_ARRAY_IS_MAPPED:
      return "CUDA_ERROR_ARRAY_IS_MAPPED";
    case CUDA_ERROR_ALREADY_MAPPED:
      return "CUDA_ERROR_ALREADY_MAPPED";
    case CUDA_ERROR_NO_BINARY_FOR_GPU:
      return "CUDA_ERROR_NO_BINARY_FOR_GPU";
    case CUDA_ERROR_ALREADY_ACQUIRED:
      return "CUDA_ERROR_ALREADY_ACQUIRED";
    case CUDA_ERROR_NOT_MAPPED:
      return "CUDA_ERROR_NOT_MAPPED";
    case CUDA_ERROR_NOT_MAPPED_AS_ARRAY:
      return "CUDA_ERROR_NOT_MAPPED_AS_ARRAY";
    case CUDA_ERROR_NOT_MAPPED_AS_POINTER:
      return "CUDA_ERROR_NOT_MAPPED_AS_POINTER";
    case CUDA_ERROR_ECC_UNCORRECTABLE:
      return "CUDA_ERROR_ECC_UNCORRECTABLE";
    case CUDA_ERROR_UNSUPPORTED_LIMIT:
      return "CUDA_ERROR_UNSUPPORTED_LIMIT";
    case CUDA_ERROR_INVALID_SOURCE:
      return "CUDA_ERROR_INVALID_SOURCE";
    case CUDA_ERROR_FILE_NOT_FOUND:
      return "CUDA_ERROR_FILE_NOT_FOUND";
    case CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND:
      return "CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND";
    case CUDA_ERROR_SHARED_OBJECT_INIT_FAILED:
      return "CUDA_ERROR_SHARED_OBJECT_INIT_FAILED";
    case CUDA_ERROR_OPERATING_SYSTEM:
      return "CUDA_ERROR_OPERATING_SYSTEM";
    case CUDA_ERROR_INVALID_HANDLE:
      return "CUDA_ERROR_INVALID_HANDLE";
    case CUDA_ERROR_NOT_FOUND:
      return "CUDA_ERROR_NOT_FOUND";
    case CUDA_ERROR_NOT_READY:
      return "CUDA_ERROR_NOT_READY";
    case CUDA_ERROR_LAUNCH_FAILED:
      return "CUDA_ERROR_LAUNCH_FAILED";
    case CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES:
      return "CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES";
    case CUDA_ERROR_LAUNCH_TIMEOUT:
      return "CUDA_ERROR_LAUNCH_TIMEOUT";
    case CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING:
      return "CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING";
    case CUDA_ERROR_UNKNOWN:
      return "CUDA_ERROR_UNKNOWN";
    default:
      return "Unknown";
  }
}

#define checkCudaErrors(err)  __checkCudaErrors (err, __FILE__, __LINE__)

#define __checkCudaErrors( err, file, line )                                  \
  do {                                                                        \
    if( CUDA_SUCCESS != err) {                                                \
      fprintf(stderr,                                                         \
              "CUDA Driver API error = %04d <%s> from file <%s>, line %i.\n", \
              err, cuHelperErrorToString(err), file, line );                  \
      throw 1;                                                                \
    }                                                                         \
  } while (0)

#include "cuFunction.hpp"

class cuHelper {

private:

  CUdevice   device;
  CUcontext  context;

  long int memUsage;
  size_t   totalGlobalMem;

  int warpSize;

public:

  cuHelper(int plat, int dev){

    CUresult err = cuInit(0);
    int deviceCount = 0;

    if (err == CUDA_SUCCESS)
      checkCudaErrors(cuDeviceGetCount(&deviceCount));

    if (deviceCount == 0) {
      fprintf(stderr, "Error: no devices supporting CUDA\n");
      throw 1;
    }

    // get device
    cuDeviceGet(&device, dev);

    /// get device name
    char devName[BUFSIZ];
    cuDeviceGetName(devName, BUFSIZ, device);
    printf("> Using device %d: %s\n", dev, devName);

    /// print device capability
    int major, minor;
    checkCudaErrors( cuDeviceComputeCapability(&major, &minor, device) );
    printf("> GPU Device has SM %d.%d compute capability\n", major, minor);

    /// find total device memory
    checkCudaErrors( cuDeviceTotalMem(&totalGlobalMem, device) );
    printf("  Total amount of global memory:   %llu bytes\n",
           (unsigned long long)totalGlobalMem);
    printf("  64-bit Memory Address:           %s\n",
           (totalGlobalMem > (unsigned long long)4*1024*1024*1024L)?
           "YES" : "NO");

    /// create context
    err = cuCtxCreate(&context, 0, device);
    if (err != CUDA_SUCCESS) {
      fprintf(stderr, "* Error initializing the CUDA context.\n");
      cuCtxDetach(context);
      throw 1;
    }


    memUsage = 0;

    checkCudaErrors( cuDeviceGetAttribute(&warpSize, CU_DEVICE_ATTRIBUTE_WARP_SIZE, device) );
  }

  CUdevice *getDevice(){
    return &device;
  }

  CUcontext *getContext(){
    return &context;
  }

  cuFunction buildKernelFromSource(const string sourcefilename, const string functionname, string defines = "", string flags = ""){
    if(defines != ""){
      int start = 0;

      for(int i = 0; i < sourcefilename.length(); i++)
        if(sourcefilename[i] == '/')
          start = (i+1);

      int length = sourcefilename.length() - start;
      string source = sourcefilename.substr(start, length);

      char defname[BUFSIZ];

#if OCCA_MPI_ENABLED
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
      return cuFunction(&context, &device, defname, functionname.c_str(), flags.c_str());
    }
    else{
      cout << "building: " << functionname << " from " <<  sourcefilename << endl;
      return cuFunction(&context, &device, sourcefilename.c_str(), functionname.c_str(), flags.c_str());
    }
  }

  cuFunction buildKernelFromBinary(const string sourceFilename, const string functionName){
    cuFunction kernel;
    return kernel.buildFromBinary(&context, &device, sourceFilename.c_str(), functionName.c_str(), "");
  }

  CUdeviceptr createBuffer(size_t sz, void *h_a){

    CUdeviceptr d_a;
    checkCudaErrors( cuMemAlloc(&d_a, sz) );

    if(h_a)
      checkCudaErrors( cuMemcpyHtoD(d_a, h_a, sz) );

    memUsage += sz;

    return d_a;
  }


  long int reportMemoryUsage(){

    return memUsage;

  }


  void finish(){
    checkCudaErrors( cuCtxSynchronize() );
  }


  void flush(){}



  void destroyBuffer(CUdeviceptr &a){

    if(a)
      checkCudaErrors( cuMemFree(a));

  }


  void toHost(size_t sz, void *dest, CUdeviceptr &source){

    checkCudaErrors( cuMemcpyDtoH(dest, source, sz) );

  }


  void toHost(size_t offset, size_t sz, void *dest, CUdeviceptr &source){

    checkCudaErrors( cuMemcpyDtoH(dest, source + offset, sz) );

  }


  void toDevice(size_t sz, CUdeviceptr &dest, void *source){

    checkCudaErrors( cuMemcpyHtoD(dest, source, sz) );

  }

  void deviceCopy(CUdeviceptr &dest, CUdeviceptr &src,
                  size_t bytes,
                  size_t srcOffset = 0,
                  size_t destOffset = 0){
    checkCudaErrors( cuMemcpy((dest + destOffset),
			      (src + srcOffset),
			      bytes) );
  }

  void queueMarker(CUevent &ev){
    cuEventCreate(&ev, CU_EVENT_DEFAULT);
    cuEventRecord(ev, 0);
  }


  double elapsedTime(CUevent& ev_start, CUevent& ev_end){
    cuEventSynchronize(ev_end);

    float millis;
    cuEventElapsedTime(&millis, ev_start, ev_end);

    return (millis/1000.0);
  }

  int preferredWorkgroupMultiple(){
    return warpSize;
  }
};


#endif
