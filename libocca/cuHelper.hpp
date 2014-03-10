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

#define checkCudaErrors(err)  __checkCudaErrors (err, __FILE__, __LINE__)

#define __checkCudaErrors( err, file, line )                            \
  do {                                                                  \
    if( CUDA_SUCCESS != err) {                                          \
      fprintf(stderr,                                                   \
              "CUDA Driver API error = %04d from file <%s>, line %i.\n", \
              err, file, line );                                        \
      throw 1;                                                          \
    }                                                                   \
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
