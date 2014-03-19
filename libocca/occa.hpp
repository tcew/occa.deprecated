#ifndef __OCCA
#define __OCCA 1

#include <iostream>
#include <fstream>
#include <sstream>

#include <sys/types.h>
#include <unistd.h>

#ifdef OS_LINUX
#  include <stdlib.h>
#endif

#include <vector>

using namespace std;

using std::string;
using std::ofstream;

#if OCCA_MPI_ENABLED
#  include <mpi.h>
#endif

#include "occaMacros.hpp"

#if OCCA_USE_OPENCL==1
#  ifdef __APPLE__
#    include <OpenCL/OpenCl.h>
#  else
#    include <CL/cl.h>
#  endif
#endif

#include "occaPods.hpp"
#include "dummyArg.hpp"

#if OCCA_USE_OPENCL==1
#  include "clHelper.hpp"
#  include "clFunction.hpp"

void cl_list_all_devices();
#endif

#if OCCA_USE_CUDA==1
#  include "cuHelper.hpp"
#  include "cuFunction.hpp"
#endif

#if OCCA_USE_CPU==1
#  include "cpuHelper.hpp"
#  include "cpuFunction.hpp"
#endif

#include "genFunction.hpp"

typedef enum {

  None   = 0,
  OpenCL = 1,
  CUDA   = 2,
  CPU    = 4,
  All    = 7

} occaModelTypes;

struct occaEvent {

#if OCCA_USE_OPENCL==1
  cl_event clEvent;
#endif

#if OCCA_USE_CUDA==1
  CUevent  cuEvent;
#endif

#if OCCA_USE_CPU==1
  occaTime cpuEvent;
#endif
};

class occaMemory {
public:

  occaModelTypes model;
  occaModelTypes mother;

#if OCCA_USE_OPENCL==1
  clHelper    *cl;
  cl_mem      clMem;
#endif

#if OCCA_USE_CPU==1
  cpuHelper   *cpu;
  void        *cpuMem;
#endif

#if OCCA_USE_CUDA==1
  cuHelper    *cu;
  CUdeviceptr cuMem;
#endif

  size_t size;

#if OCCA_USE_OPENCL==1
#  define OCCA_MEMORY_CL_CONSTRUCTOR            \
  cl    = NULL;                                 \
  clMem = 0;
#else
#  define OCCA_MEMORY_CL_CONSTRUCTOR
#endif

#if OCCA_USE_CPU==1
#  define OCCA_MEMORY_CPU_CONSTRUCTOR           \
  cpu    = NULL;                                \
  cpuMem = 0;
#else
#  define OCCA_MEMORY_CPU_CONSTRUCTOR
#endif

#if OCCA_USE_CUDA==1
#  define OCCA_MEMORY_CU_CONSTRUCTOR            \
  cu    = NULL;                                 \
  cuMem = 0;
#else
#  define OCCA_MEMORY_CU_CONSTRUCTOR
#endif

#define OCCA_MEMORY_DEFAULT_CONSTRUCTOR         \
  model = None;                                 \
  mother = None;                                \
                                                \
  OCCA_MEMORY_CL_CONSTRUCTOR;                   \
  OCCA_MEMORY_CU_CONSTRUCTOR;                   \
  OCCA_MEMORY_CPU_CONSTRUCTOR;                  \
                                                \
  size = 0;

  occaMemory(){
    OCCA_MEMORY_DEFAULT_CONSTRUCTOR;
  }

#if OCCA_USE_OPENCL==1
  occaMemory(clHelper *_cl, size_t _size, void *data){
    OCCA_MEMORY_DEFAULT_CONSTRUCTOR;

    model = OpenCL;

    cl = _cl;

    size = _size;

    if(size==0) size = 1;
    clMem = cl->createBuffer(size, data);

    mother = model;
  }
#endif

#if OCCA_USE_CUDA==1
  occaMemory(cuHelper *_cu, size_t _size, void *data){
    OCCA_MEMORY_DEFAULT_CONSTRUCTOR;

    model = CUDA;

    cu = _cu;

    size = _size;

    if(size==0) size = 1;
    cuMem = cu->createBuffer(size, data);

    mother = model;
  }
#endif

#if OCCA_USE_CPU==1
  occaMemory(cpuHelper *_cpu, size_t _size, void *data){
    OCCA_MEMORY_DEFAULT_CONSTRUCTOR;

    model = CPU;

    cpu = _cpu;

    size = _size;

    if(size==0) size = 1;
    cpuMem = cpu->createBuffer(size, data);

    mother = model;

  }
#endif

#if OCCA_USE_ALL==1
  occaMemory(occaModelTypes _mother, clHelper *_cl, cuHelper *_cu, cpuHelper *_cpu, size_t _size, void *data){
    OCCA_MEMORY_DEFAULT_CONSTRUCTOR;

    mother = _mother;
    model = All;

    cl = _cl;
    cu = _cu;
    cpu = _cpu;

    size = _size;

    if(size==0) size = 1;

#if OCCA_USE_OPENCL==1
    if(model & OpenCL) clMem  = cl->createBuffer(size, data);
#endif

#if OCCA_USE_CUDA==1
    if(model & CUDA)   cuMem  = cu->createBuffer(size, data);
#endif

#if OCCA_USE_CPU==1
    if(model & CPU)   cpuMem = cpu->createBuffer(size, data);
#endif

  }
#endif

  void toHost(size_t sz, void *dest){

    switch(mother){

#if OCCA_USE_OPENCL==1
    case OpenCL: cl->toHost(sz, dest, clMem);   break;
#endif

#if OCCA_USE_CUDA==1
    case CUDA:   cu->toHost(sz, dest, cuMem);   break;
#endif

#if OCCA_USE_CPU==1
    case CPU:    cpu->toHost(sz, dest, cpuMem); break;
#endif

    default: cout << "Mother not valid [1]\n"; throw 1;
    }
  }

  void toHost(size_t offset, size_t sz, void *dest){
    switch(mother){

#if OCCA_USE_OPENCL==1
    case OpenCL: cl->toHost(offset, sz, dest, clMem);   break;
#endif

#if OCCA_USE_CUDA==1
    case CUDA:   cu->toHost(offset, sz, dest, cuMem);   break;
#endif

#if OCCA_USE_CPU==1
    case CPU:    cpu->toHost(offset, sz, dest, cpuMem); break;
#endif
    default: cout << "Mother not valid\n"; throw 1;
    }
  }

  void toHost(void *dest){

    switch(mother){

#if OCCA_USE_OPENCL==1
    case OpenCL: cl->toHost(size, dest, clMem);
      break;
#endif

#if OCCA_USE_CUDA==1
    case CUDA:   cu->toHost(size, dest, cuMem);
      break;
#endif

#if OCCA_USE_CPU==1
    case CPU:    cpu->toHost(size, dest, cpuMem);
      break;
#endif
    default: cout << "Mother not valid [2]\n"; throw 1;
    }

  }

  template <class T>
  void toHost(vector<T> &v){
    if(v.size() == 0)
      return;

    switch(mother){

#if OCCA_USE_OPENCL==1
    case OpenCL: cl->toHost(v.size()*sizeof(T), &(v[0]), clMem);   break;
#endif

#if OCCA_USE_CUDA==1
    case CUDA:   cu->toHost(v.size()*sizeof(T), &(v[0]), cuMem);   break;
#endif

#if OCCA_USE_CPU==1
    case CPU:    cpu->toHost(v.size()*sizeof(T), &(v[0]), cpuMem); break;
#endif

    default: cout << "Mother not valid [1]\n"; throw 1;
    }
  }

  void toDevice(void *source){

#if OCCA_USE_OPENCL==1
    if(model & OpenCL)  cl->toDevice(size,  clMem, source);
#endif

#if OCCA_USE_CUDA==1
    if(model & CUDA)    cu->toDevice(size,  cuMem, source);
#endif

#if OCCA_USE_CPU==1
    if(model & CPU)    cpu->toDevice(size, cpuMem, source);
#endif

  }

  void toDevice(size_t sz, void *source){

#if OCCA_USE_OPENCL==1
    if(model & OpenCL)  cl->toDevice(sz, clMem, source);
#endif

#if OCCA_USE_CUDA==1
    if(model & CUDA)    cu->toDevice(sz, cuMem, source);
#endif

#if OCCA_USE_CPU==1
    if(model & CPU)    cpu->toDevice(sz, cpuMem, source);
#endif
  }

  template <class T>
  void toDevice(vector<T> &v){
    if(v.size() == 0)
      return;

#if OCCA_USE_OPENCL==1
    if(model & OpenCL)  cl->toDevice(v.size()*sizeof(T),  clMem, &(v[0]));
#endif

#if OCCA_USE_CUDA==1
    if(model & CUDA)    cu->toDevice(v.size()*sizeof(T),  cuMem , &(v[0]));
#endif

#if OCCA_USE_CPU==1
    if(model & CPU)    cpu->toDevice(v.size()*sizeof(T), cpuMem, &(v[0]));
#endif
  }

  void copyTo(occaMemory &mem){
#if OCCA_USE_OPENCL==1
    if(model & OpenCL) cl->deviceCopy(mem.clMem, clMem, size);
#endif

#if OCCA_USE_CUDA==1
    if(model & CUDA)   cu->deviceCopy(mem.cuMem, cuMem, size);
#endif

#if OCCA_USE_CPU==1
    if(model & CPU)    cpu->deviceCopy(mem.cpuMem, cpuMem, size);
#endif
  }

  void swap(occaMemory &mem){
    occaModelTypes model_ = mem.model;
    mem.model = model;
    model     = model_;

    occaModelTypes mother_ = mem.mother;
    mem.mother = mother;
    mother     = mother_;

#if OCCA_USE_OPENCL==1
    clHelper *cl_ = mem.cl;
    mem.cl = cl;
    cl     = cl_;

    cl_mem clMem_ = mem.clMem;
    mem.clMem = clMem;
    clMem     = clMem_;
#endif

#if OCCA_USE_CPU==1
    cpuHelper *cpu_ = mem.cpu;
    mem.cpu = cpu;
    cpu     = cpu_;

    void *cpuMem_ = mem.cpuMem;
    mem.cpuMem = cpuMem;
    cpuMem     = cpuMem_;

#endif

#if OCCA_USE_CUDA==1
    cuHelper *cu_ = mem.cu;
    mem.cu = cu;
    cu     = cu_;

    CUdeviceptr cuMem_ = mem.cuMem;
    mem.cuMem = cuMem;
    cuMem     = cuMem_;
#endif

    size_t size_ = mem.size;
    mem.size = size;
    size     = size_;
  }
};


class occaKernel {
private:
  occaModelTypes model;
  occaModelTypes mother;

#if OCCA_USE_OPENCL==1
  clHelper   *cl;
  clFunction clfn;
#endif

#if OCCA_USE_CUDA==1
  cuHelper   *cu;
  cuFunction cufn;
#endif

#if OCCA_USE_CPU==1
  cpuHelper   *cpu;
  cpuFunction cpufn;
#endif

  int global[3], local[3];
public:

  double bandWidth;
  int numCalls;

  occaKernel(){

    model = None;
    mother = None;

    bandWidth = 0.;
    numCalls = 0;

#if OCCA_USE_OPENCL==1
    cl = NULL;
#endif

#if OCCA_USE_CPU==1
    cpu = NULL;
#endif

#if OCCA_USE_CUDA==1
    cu = NULL;
#endif
  }

#if OCCA_USE_OPENCL==1
  occaKernel(clHelper *_cl, string filename, string kernelname, string defines, string flags){
    buildKernelFromSource(_cl, filename, kernelname, defines, flags);
  }
#endif

#if OCCA_USE_CUDA==1
  occaKernel(cuHelper *_cu, string filename, string kernelname, string defines, string flags){
    buildKernelFromSource(_cu, filename, kernelname, defines, flags);
  }
#endif

#if OCCA_USE_CPU==1
  occaKernel(cpuHelper *_cpu, string filename, string kernelname, string defines, string flags){
    buildKernelFromSource(_cpu, filename, kernelname, defines, flags);
  }
#endif

#if OCCA_USE_ALL==1
  occaKernel(occaModelTypes _mother,  clHelper *_cl, cuHelper *_cu, cpuHelper *_cpu, string filename, string kernelname, string defines, string flags){
    buildKernelFromSource(_mother, _cl, _cu, _cpu, filename, kernelname, defines, flags);
  }
#endif

  occaKernel(const occaKernel &k){
    *this = k;
  }

  occaKernel& operator = (const occaKernel &k){
    model  = k.model;
    mother = k.mother;

#if OCCA_USE_OPENCL==1
    cl   = k.cl;
    clfn = k.clfn;
#endif

#if OCCA_USE_CPU==1
    cpu   = k.cpu;
    cpufn = k.cpufn;
#endif

#if OCCA_USE_CUDA==1
    cu   = k.cu;
    cufn  = k.cufn;
#endif

    for(int i = 0; i < 3; i++){
      global[i] = k.global[i];
      local[i]  = k.local[i];
    }

    bandWidth = k.bandWidth;
    numCalls = k.numCalls;

    return *this;
  }

  //---[ Source Kernel ]----------------

#if OCCA_USE_OPENCL==1
  occaKernel& buildKernelFromSource(clHelper *_cl, string filename, string kernelname, string defines, string flags){

    model = OpenCL;

    cl = _cl;

    bandWidth = 0.;
    numCalls = 0;

    defines = "#include \"libocca/occaCLdefines.hpp\"\n" + defines;

    clfn = cl->buildKernelFromSource(filename, kernelname, defines, flags);

#if OCCA_USE_CPU==1
    cpu = NULL;
#endif

#if OCCA_USE_CUDA==1
    cu = NULL;
#endif

    mother = model;

    return *this;
  }
#endif


#if OCCA_USE_CUDA==1
  occaKernel& buildKernelFromSource(cuHelper *_cu, string filename, string kernelname, string defines, string flags){

    model = CUDA;

    cu = _cu;

    bandWidth = 0.;
    numCalls = 0;

    defines = "#include \"libocca/occaCUdefines.hpp\"\n" + defines;

    cufn = cu->buildKernelFromSource(filename, kernelname, defines, flags);

#if OCCA_USE_CPU==1
    cpu = NULL;
#endif

#if OCCA_USE_OPENCL==1
    cl = NULL;
#endif

    mother = model;

    return *this;
  }
#endif

#if OCCA_USE_CPU==1
  occaKernel& buildKernelFromSource(cpuHelper *_cpu, string filename, string kernelname, string defines, string flags){

    model = CPU;

    cpu = _cpu;

    bandWidth = 0.;
    numCalls = 0;

#ifndef OCCA_DO_NOT_USE_OPENMP
    defines = "#include <omp.h>\n" + defines;
#endif

    defines = "#include \"libocca/occaCPUdefines.hpp\"\n" + defines;

    cpufn = cpu->buildKernelFromSource(filename, kernelname, defines, flags);

#if OCCA_USE_CUDA==1
    cu = NULL;
#endif

#if OCCA_USE_OPENCL==1
    cl = NULL;
#endif

    mother = model;
    return *this;
  }
#endif

#if OCCA_USE_ALL==1
  occaKernel& buildKernelFromSource(occaModelTypes _mother,  clHelper *_cl, cuHelper *_cu, cpuHelper *_cpu, string filename, string kernelname, string defines, string flags){

    model = All;
    mother = _mother;

    bandWidth = 0.;
    numCalls = 0;

#if OCCA_USE_OPENCL==1
    cl  = _cl;
    if(model & OpenCL) clfn  =  cl->buildKernelFromSource(filename, kernelname, defines, flags);
#endif

#if OCCA_USE_CUDA==1
    cu  = _cu;
    if(model & CUDA)   cufn  =  cu->buildKernelFromSource(filename, kernelname, defines, flags);
#endif

#if OCCA_USE_CPU==1
    cpu = _cpu;
    if(model & CPU)    cpufn = cpu->buildKernelFromSource(filename, kernelname, defines, flags);
#endif

    return *this;
  }
#endif

  //---[ Binary Kernel ]----------------

#if OCCA_USE_OPENCL==1
  occaKernel& buildKernelFromBinary(clHelper *_cl, string filename, string kernelname){

    model = OpenCL;

    cl = _cl;

    bandWidth = 0.;
    numCalls = 0;

    clfn = cl->buildKernelFromBinary(filename, kernelname);

#if OCCA_USE_CPU==1
    cpu = NULL;
#endif

#if OCCA_USE_CUDA==1
    cu = NULL;
#endif

    mother = model;

    return *this;
  }
#endif


#if OCCA_USE_CUDA==1
  occaKernel& buildKernelFromBinary(cuHelper *_cu, string filename, string kernelname){

    model = CUDA;

    cu = _cu;

    bandWidth = 0.;
    numCalls = 0;

    cufn = cu->buildKernelFromBinary(filename, kernelname);

#if OCCA_USE_CPU==1
    cpu = NULL;
#endif

#if OCCA_USE_OPENCL==1
    cl = NULL;
#endif

    mother = model;
    return *this;
  }
#endif

#if OCCA_USE_CPU==1
  occaKernel& buildKernelFromBinary(cpuHelper *_cpu, string filename, string kernelname){

    model = CPU;

    cpu = _cpu;

    bandWidth = 0.;
    numCalls = 0;

    cpufn = cpu->buildKernelFromBinary(filename, kernelname);

#if OCCA_USE_CUDA==1
    cu = NULL;
#endif

#if OCCA_USE_OPENCL==1
    cl = NULL;
#endif

    mother = model;
    return *this;
  }
#endif

#if OCCA_USE_ALL==1
  occaKernel& buildKernelFromBinary(occaModelTypes _mother,  clHelper *_cl, cuHelper *_cu, cpuHelper *_cpu, string filename, string kernelname){

    model = All;
    mother = _mother;

    bandWidth = 0.;
    numCalls = 0;

#if OCCA_USE_OPENCL==1
    cl  = _cl;
    if(model & OpenCL) clfn  =  cl->buildKernelFromBinary(filename, kernelname);
#endif

#if OCCA_USE_CUDA==1
    cu  = _cu;
    if(model & CUDA)   cufn  =  cu->buildKernelFromBinary(filename, kernelname);
#endif

#if OCCA_USE_CPU==1
    cpu = _cpu;
    if(model & CPU)    cpufn = cpu->buildKernelFromBinary(filename, kernelname);
#endif

    return *this;
  }
#endif

  void setThreadArray(size_t *_global, size_t *_local, int _dim){
#if OCCA_USE_OPENCL==1
    if(model & OpenCL) clfn.setThreadArray(_global, _local, _dim);
#endif

#if OCCA_USE_CUDA==1
    if(model & CUDA)   cufn.setThreadArray(_global, _local, _dim);
#endif

#if OCCA_USE_CPU==1
    if(model & CPU)   cpufn.setThreadArray(_global, _local, _dim);
#endif
  }

  void setThreadDims(int dim, occaDim groups, occaDim items){
#if OCCA_USE_OPENCL==1
    if(model & OpenCL) clfn.setThreadDims(dim, groups, items);
#endif

#if OCCA_USE_CUDA==1
    if(model & CUDA)   cufn.setThreadDims(dim, groups, items);
#endif

#if OCCA_USE_CPU==1
    if(model & CPU)   cpufn.setThreadDims(dim, groups, items);
#endif
  }

  OCCA_KERNEL_OPERATORS;

  void enqueue(int argc, void* args[], size_t argssz[]){
    void *local_args[OCCA_MAX_NUM_ARGS];

    for(int i = 0; i < argc; ++i)
    {
      local_args[i] = args[i];
    }

#if OCCA_USE_OPENCL==1
    for(int i = 0; i < argc; ++i)
    {
      if(argssz[i] == 0)
      {
        occaMemory * mem = (occaMemory *) args[i];
        local_args[i] = (void*)&mem->clMem;
      }
    }
    if(model & OpenCL) clfn.enqueue(argc, local_args, argssz);
#endif

#if OCCA_USE_CUDA==1
    for(int i = 0; i < argc; ++i)
    {
      if(argssz[i] == 0)
      {
        occaMemory * mem = (occaMemory *) args[i];
        local_args[i] = (void*)&mem->cuMem;
      }
    }
    if(model & CUDA)   cufn.enqueue(argc, local_args, argssz);
#endif

#if OCCA_USE_CPU==1
    for(int i = 0; i < argc; ++i)
    {
      if(argssz[i] == 0)
      {
        occaMemory * mem = (occaMemory *) args[i];
        local_args[i] = (void*)mem->cpuMem;
      }
    }
    if(model & CPU)   cpufn.enqueue(argc, local_args, argssz);
#endif

  }

  double elapsed(){

#if OCCA_USE_OPENCL==1
    if(mother == OpenCL) return clfn.elapsedTime();
#endif

#if OCCA_USE_CUDA==1
    if(mother == CUDA)   return cufn.elapsedTime();
#endif

#if OCCA_USE_CPU==1
    if(mother == CPU)    return cpufn.elapsedTime();
#endif

    return 0;
  }

  void tic(){
#if OCCA_USE_OPENCL==1
    if(mother == OpenCL) clfn.tic();
#endif

#if OCCA_USE_CUDA==1
    if(mother == CUDA)   cufn.tic();
#endif

#if OCCA_USE_CPU==1
    if(mother == CPU)    cpufn.tic();
#endif
  }

  void toc(){
#if OCCA_USE_OPENCL==1
    if(mother == OpenCL) clfn.toc();
#endif

#if OCCA_USE_CUDA==1
    if(mother == CUDA)   cufn.toc();
#endif

#if OCCA_USE_CPU==1
    if(mother == CPU)    cpufn.toc();
#endif
  }

  int preferredWorkgroupMultiple(){
    return 1;
  }

  double getBandWidth(){

    if(numCalls)
      return (1e-9)*bandWidth/numCalls;
    else
      return 0.0;
  }


};



class occa {
private:

#if OCCA_USE_OPENCL==1
  clHelper *cl;
#endif

#if OCCA_USE_CPU==1
  cpuHelper *cpu;
#endif

#if OCCA_USE_CUDA==1
  cuHelper *cu;
#endif

  occaModelTypes model;
  occaModelTypes mother;

  int memoryCount;

public:

  occa(){

    memoryCount = 0;

#if OCCA_USE_OPENCL==1
    cl = NULL;
#endif

#if OCCA_USE_CPU==1
    cpu = NULL;
#endif

#if OCCA_USE_CUDA==1
    cu  = NULL;
#endif

    model = None;
    mother = None;
  }

  int &memCount(){
    return memoryCount;
  }


  inline occa& operator = (const occa &o){
    //  void operator = (const occa &o){
#if OCCA_USE_OPENCL==1
    cl = o.cl;
#endif

#if OCCA_USE_CPU==1
    cpu = o.cpu;
#endif

#if OCCA_USE_CUDA==1
    cu = o.cu;
#endif

    model = o.model;
    mother = o.mother;

    memoryCount = o.memoryCount;

    return *this;
  }

  occaModelTypes interpretModel(string modelString){

#if OCCA_USE_OPENCL==1
    if(modelString == "OpenCL"){
      return OpenCL;
    }
#endif

#if OCCA_USE_CUDA==1
    if(modelString == "CUDA"){
      return CUDA;
    }
#endif

#if OCCA_USE_CPU==1
    if(modelString == "CPU"){
      return CPU;
    }
#endif

#if OCCA_USE_ALL==1
    if(modelString == "All"){
      return All;
    }
#endif

    return None;
  }

  void setup(string threadModel, string threadMother, int _platform, int _device){

    model = All;
    mother = interpretModel(threadMother);

#if OCCA_USE_OPENCL==1
    if(model & OpenCL) cl  = new clHelper (_platform, _device);
#else
    if(model & OpenCL) { cout << "Exiting: OpenCL not available" << endl; throw 1; }
#endif

#if OCCA_USE_CUDA==1
    if(model & CUDA)   cu  = new cuHelper (_platform, _device);
#else
    if(model & CUDA) { cout << "Exiting: CUDA not available" << endl; throw 1; }
#endif

#if OCCA_USE_CPU==1
    if(model & CPU)    cpu = new cpuHelper(_platform, _device);
#else
    if(model & CPU) { cout << "Exiting: CPU not available" << endl; throw 1; }
#endif
  }

  inline bool isUsingCPU(){
#if OCCA_USE_OPENCL==1
    if(model == OpenCL) return false;
#endif

#if OCCA_USE_CUDA==1
    if(model == CUDA)   return false;
#endif

#if OCCA_USE_CPU==1
    if(model == CPU)    return true;
#endif

#if OCCA_USE_ALL==1
    if(model == All)    return true;
#endif

    return false; // Won't get here
  }

  inline bool isUsingGPU(){
#if OCCA_USE_OPENCL==1
    if(model == OpenCL) return true;
#endif

#if OCCA_USE_CUDA==1
    if(model == CUDA)   return true;
#endif

#if OCCA_USE_CPU==1
    if(model == CPU)    return false;
#endif

#if OCCA_USE_ALL==1
    if(model == All)    return false;
#endif

    return false; // Won't get here
  }

  void setup(string threadModel, int _platform, int _device){

#if OCCA_USE_OPENCL==1
    if(threadModel == "OpenCL"){
      cl = new clHelper(_platform, _device);
      model = OpenCL;
    }
#else
    if(threadModel == "OpenCL") { cout << "Exiting: OpenCL not available" << endl; throw 1; }
#endif

#if OCCA_USE_CPU==1
    if(threadModel == "CPU"){
      cpu = new cpuHelper(_platform, _device);
      model = CPU;
    }
#else
    if(threadModel == "CPU") { cout << "Exiting: CPU not available" << endl; throw 1; }
#endif

#if OCCA_USE_CUDA==1
    if(threadModel == "CUDA"){
      cu = new cuHelper(_platform, _device);
      model = CUDA;
    }
#else
    if(threadModel == "CUDA") { cout << "Exiting: CUDA not available" << endl; throw 1; }
#endif

    mother = model;

  }

  occaKernel &buildKernel(string filename, string kernelname, string defFile = "", string flags = ""){

    occaKernel *fn;

    switch(model){
#if OCCA_USE_OPENCL==1
    case OpenCL:  fn = new occaKernel(cl,  filename, kernelname, defFile, flags); break;
#endif

#if OCCA_USE_CUDA==1
    case CUDA:    fn = new occaKernel(cu,  filename, kernelname, defFile, flags); break;
#endif

#if OCCA_USE_CPU==1
    case CPU:     fn = new occaKernel(cpu, filename, kernelname, defFile, flags); break;
#endif

#if OCCA_USE_ALL==1
    case All:     fn = new occaKernel(mother, cl, cu, cpu, filename, kernelname, defFile, flags); break;
#endif
    default: cout << "Model not valid\n"; throw 1;
    }

    return *fn;

  }

  occaKernel buildKernelFromBinary(string filename, string kernelname){
    occaKernel fn;

    switch(model){
#if OCCA_USE_OPENCL==1
    case OpenCL:  fn.buildKernelFromBinary(cl,  filename, kernelname); break;
#endif

#if OCCA_USE_CUDA==1
    case CUDA:    fn.buildKernelFromBinary(cu,  filename, kernelname); break;
#endif

#if OCCA_USE_CPU==1
    case CPU:     fn.buildKernelFromBinary(cpu, filename, kernelname); break;
#endif

#if OCCA_USE_ALL==1
    case All:     fn.buildKernelFromBinary(mother, cl, cu, cpu, filename, kernelname); break;
#endif
    default: cout << "Model not valid\n"; throw 1;
    }

    return fn;
  }

  occaMemory &createBuffer(size_t sz, void *data = NULL){

    occaMemory *mem;

    switch(model){
#if OCCA_USE_OPENCL==1
    case OpenCL: mem = new occaMemory(cl,  sz, data); break;
#endif

#if OCCA_USE_CUDA==1
    case CUDA:   mem = new occaMemory(cu,  sz, data); break;
#endif

#if OCCA_USE_CPU==1
    case CPU:    mem = new occaMemory(cpu, sz, data); break;
#endif

#if OCCA_USE_ALL==1
    case All:    mem = new occaMemory(mother, cl, cu, cpu, sz, data); break;
#endif
    default: cout << "Model not valid [5]\n"; throw 1;
    }

    return *mem;

  }

  template <class T>
  occaMemory &createBuffer(vector<T> &v){
    occaMemory *mem;

    switch(model){
#if OCCA_USE_OPENCL==1
    case OpenCL: mem = new occaMemory(cl, v.size()*sizeof(T), &(v[0])); break;
#endif

#if OCCA_USE_CUDA==1
    case CUDA:   mem = new occaMemory(cu, v.size()*sizeof(T), &(v[0])); break;
#endif

#if OCCA_USE_CPU==1
    case CPU:    mem = new occaMemory(cpu, v.size()*sizeof(T), &(v[0])); break;
#endif

#if OCCA_USE_ALL==1
    case All:    mem = new occaMemory(mother, cl, cu, cpu, v.size()*sizeof(T), &(v[0])); break;
#endif
    default: cout << "Model not valid [5]\n"; throw 1;
    }

    return *mem;
  }


  void destroyBuffer(occaMemory &a){

#if OCCA_USE_OPENCL==1
    if(model & OpenCL)  cl->destroyBuffer(a.clMem);
#endif

#if OCCA_USE_CUDA==1
    if(model & CUDA)    cu->destroyBuffer(a.cuMem);
#endif

#if OCCA_USE_CPU==1
    if(model & CPU)     cpu->destroyBuffer(a.cpuMem);
#endif
  }

  char *compilerString(const char *st){

    char *buf = (char*) calloc(BUFSIZ,sizeof(char));
    switch(model){
#if OCCA_USE_OPENCL==1
    case OpenCL:
      sprintf(buf, "%s", st);
      break;
#endif

#if OCCA_USE_CPU==1
    case CPU:
      sprintf(buf, "'%s'", st);
      break;
#endif

#if OCCA_USE_CUDA==1
    case CUDA:
      sprintf(buf, "'%s'", st);
      break;
#endif

    default: cout << "Model not valid [6]\n"; throw 1;
    }

    return buf;
  }

  void queueMarker(occaEvent &event){
    switch(model){
#if OCCA_USE_OPENCL==1
    case OpenCL: cl->queueMarker(event.clEvent);   break;
#endif

#if OCCA_USE_CUDA==1
    case CUDA:   cu->queueMarker(event.cuEvent);   break;
#endif

#if OCCA_USE_CPU==1
    case CPU:    cpu->queueMarker(event.cpuEvent); break;
#endif

    default: cout << "Model not valid [7]\n"; throw 1;
    }
  }

  double elapsedTime(occaEvent &start, occaEvent &end){
    switch(model){
#if OCCA_USE_OPENCL==1
    case OpenCL: return cl->elapsedTime(start.clEvent, end.clEvent);
#endif

#if OCCA_USE_CUDA==1
    case CUDA:   return cu->elapsedTime(start.cuEvent, end.cuEvent);
#endif

#if OCCA_USE_CPU==1
    case CPU:    return cpu->elapsedTime(start.cpuEvent, end.cpuEvent);
#endif
    default: cout << "Model not valid [8]\n"; throw 1;
    }

    return 0;
  }

  inline int preferredWorkgroupMultiple(){
    switch(model){
#if OCCA_USE_OPENCL==1
    case OpenCL: return cl->preferredWorkgroupMultiple() ; break;
#endif

#if OCCA_USE_CUDA==1
    case CUDA:   return cu->preferredWorkgroupMultiple() ; break;
#endif

#if OCCA_USE_CPU==1
    case CPU:    return cpu->preferredWorkgroupMultiple(); break;
#endif

#if OCCA_USE_ALL==1
    case All:    return cl->preferredWorkgroupMultiple() ; break; // For now OpenCL's workgroup size
#endif

    default: cout << "Model not valid\n"; throw 1;
    }

    return 0;
  }

  void finish(){
    switch(model){
#if OCCA_USE_OPENCL==1
    case OpenCL: cl->finish(); break;
#endif

#if OCCA_USE_CUDA==1
    case CUDA:   cu->finish(); break;
#endif

#if OCCA_USE_CPU==1
    case CPU:    cpu->finish(); break;
#endif

    default: cout << "Model not valid [9]\n"; throw 1;
    }
  }

  inline void addInclude(string &str, string includeFile){
    stringstream ss;
    ss << "#include \"" << includeFile << "\"\n";

    // Defines are appended
    str += ss.str();
  }

  template <class T>
  inline void addDefine(string &str, string name, const T &value){
    stringstream ss;
    ss << "#define " << name << " " << value << "\n";

    // Defines are pre-pended
    str = ss.str() + str;
  }

};
#endif
