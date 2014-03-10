#ifndef __CPUHELPER
#define __CPUHELPER

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

// void cl_build_kernel(cl_context context, cl_device_id device,
// 		     string filename, string flags, cl_kernel *kernel);

#include "cpuFunction.hpp"

class cpuHelper {

private:

  /* set up CPU */
  long int memUsage;

public:

  cpuHelper(int plat, int dev){
    memUsage = 0;
  }

  cpuFunction buildKernel(const string sourcefilename, const string functionname, string defines = "", string flags = ""){
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
      return cpuFunction(defname, functionname.c_str(), flags.c_str());
    }
    else{
      cout << "building: " << functionname << " from " <<  sourcefilename << endl;
      return cpuFunction(sourcefilename, functionname, flags);
    }
  }

  void *createBuffer(size_t sz, void *data){

    memUsage += sz;

    void *buf = (void*) malloc(sz);
    //    void *buf;
    //    posix_memalign(&buf, 16, sz);
    if(data != NULL)
      memcpy(buf, data, sz);

    return buf;
  }


  long int reportMemoryUsage(){

    return memUsage;

  }

  void finish(){}

  void flush(){}

  void destroyBuffer(void *a){

    if(a)
      free(a);
  }


  void toHost(size_t sz, void *dest, void *source){

    memcpy(dest, source, sz);

  }

  void toHost(size_t offset, size_t sz, void *dest, void *source){

    memcpy(dest, (void*) ((char*) source + offset), sz);

  }

  void toDevice(size_t sz, void *dest, void *source){

    memcpy(dest, source, sz);

  }

  void deviceCopy(void *dest, void *src,
                  size_t bytes,
                  size_t destOffset = 0,
                  size_t srcOffset  = 0){
    memcpy((void*) ((char*) dest + destOffset),
           (void*) ((char*) src + srcOffset),
           bytes);
  }


  void queueMarker(occaTime &ev){
#ifdef        OS_LINUX
    clock_gettime(CLOCK_MONOTONIC, &ev);
#elif defined OS_OSX
    ev = mach_absolute_time();
#elif defined OS_WINDOWS
#  error "Not tested on Windows OS yet."
#endif
  }

  double elapsedTime(occaTime ev_start, occaTime ev_end){
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

  // For now just 1
  int preferredWorkgroupMultiple(){
    return 1;
  }

};


#endif