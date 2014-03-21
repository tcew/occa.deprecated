#if OCCA_CUDA_ENABLED

#include "occaCUDA.hpp"

namespace occa {
  //---[ Kernel ]---------------------
  template <>
  kernel_t<CUDA>::kernel_t(){
    data = NULL;
    dev  = NULL;

    functionName = "";

    dims  = 1;
    inner = occa::dim(1,1,1);
    outer = occa::dim(1,1,1);

    preferredDimSize_ = 0;

    startTime = (void*) new CUevent;
    endTime   = (void*) new CUevent;
  }

  template <>
  kernel_t<CUDA>::kernel_t(const kernel_t<CUDA> &k){
    data = k.data;
    dev  = k.dev;

    functionName = k.functionName;

    dims  = k.dims;
    inner = k.inner;
    outer = k.outer;

    preferredDimSize_ = k.preferredDimSize_;

    startTime = k.startTime;
    endTime   = k.endTime;
  }

  template <>
  kernel_t<CUDA>& kernel_t<CUDA>::operator = (const kernel_t<CUDA> &k){
    data = k.data;
    dev  = k.dev;

    functionName = k.functionName;

    dims  = k.dims;
    inner = k.inner;
    outer = k.outer;

    *((CUevent*) startTime) = *((CUevent*) k.startTime);
    *((CUevent*) endTime)   = *((CUevent*) k.endTime);

    return *this;
  }

  template <>
  kernel_t<CUDA>::~kernel_t(){}

  template <>
  kernel_t<CUDA>& kernel_t<CUDA>::buildFromSource(const std::string &filename,
                                                  const std::string &functionName_,
                                                  const kernelInfo &info_){
    OCCA_EXTRACT_DATA(CUDA, Kernel);

    functionName = functionName_;

    kernelInfo info = info_;
    info.addDefine("OCCA_USING_CPU", 0);
    info.addDefine("OCCA_USING_GPU", 1);

    info.addDefine("OCCA_USING_OPENMP", 0);
    info.addDefine("OCCA_USING_OPENCL", 0);
    info.addDefine("OCCA_USING_CUDA"  , 1);

    info.addOCCAKeywords(occaCUDADefines);

    std::string cachedBinary = binaryIsCached(filename, "CUDA" + info.salt());

    if(fileExists(cachedBinary)){
      std::cout << "Found cached binary of [" << filename << "] in [" << cachedBinary << "]\n";
      return buildFromBinary(cachedBinary, functionName);
    }

    std::string iCachedBinary = createIntermediateSource(filename,
                                                         cachedBinary,
                                                         info);

    std::stringstream command;

    command << dev->cudaCompiler
            << " -o "       << cachedBinary
            << " -ptx -I."
            << " -arch=sm_" << dev->cudaArch
            << ' '          << dev->cudaCompilerFlags
            << ' '          << info.flags
            << " -x cu "    << iCachedBinary;

    const std::string &sCommand = command.str();

    std::cout << sCommand << '\n';

    system(sCommand.c_str());

    OCCA_CUDA_CHECK("Kernel (" + functionName + ") : Loading Module",
                    cuModuleLoad(&data_.module, cachedBinary.c_str()));

    OCCA_CUDA_CHECK("Kernel (" + functionName + ") : Loading Function",
                    cuModuleGetFunction(&data_.function, data_.module, functionName.c_str()));

    return *this;
  }

  template <>
  kernel_t<CUDA>& kernel_t<CUDA>::buildFromBinary(const std::string &filename,
                                                  const std::string &functionName_){
    OCCA_EXTRACT_DATA(CUDA, Kernel);

    functionName = functionName_;

    OCCA_CUDA_CHECK("Kernel (" + functionName + ") : Loading Module",
                    cuModuleLoad(&data_.module, filename.c_str()));

    OCCA_CUDA_CHECK("Kernel (" + functionName + ") : Loading Function",
                    cuModuleGetFunction(&data_.function, data_.module, functionName.c_str()));

    return *this;
  }

  template <>
  int kernel_t<CUDA>::preferredDimSize(){
    preferredDimSize_ = 32;
    return 32;
  }

  OCCA_CUDA_KERNEL_OPERATOR_DEFINITIONS;

  template <>
  double kernel_t<CUDA>::timeTaken(){
    CUevent &startEvent = *((CUevent*) startTime);
    CUevent &endEvent   = *((CUevent*) endTime);

    cuEventSynchronize(endEvent);

    float msTimeTaken;
    cuEventElapsedTime(&msTimeTaken, startEvent, endEvent);

    return 1.0e-3*msTimeTaken;
  }

  template <>
  void kernel_t<CUDA>::free(){
  }
  //==================================


  //---[ Memory ]---------------------
  template <>
  memory_t<CUDA>::memory_t(){
    handle = NULL;
    dev    = NULL;
    size = 0;
  }

  template <>
  memory_t<CUDA>::memory_t(const memory_t<CUDA> &m){
    handle = m.handle;
    dev    = m.dev;
    size   = m.size;
  }

  template <>
  memory_t<CUDA>& memory_t<CUDA>::operator = (const memory_t<CUDA> &m){
    handle = m.handle;
    dev    = m.dev;
    size   = m.size;

    return *this;
  }

  template <>
  memory_t<CUDA>::~memory_t(){}

  template <>
  void memory_t<CUDA>::copyFrom(const void *source,
                                const size_t bytes,
                                const size_t offset){
    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CUDA_CHECK("Memory: Copy From",
                    cuMemcpyHtoD(*((CUdeviceptr*) handle) + offset, source, bytes_) );
  }

  template <>
  void memory_t<CUDA>::copyFrom(const memory_v *source,
                                const size_t bytes,
                                const size_t offset){
    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CUDA_CHECK("Memory: Copy From",
                    cuMemcpyDtoD(*((CUdeviceptr*) handle) + offset, *((CUdeviceptr*) source->handle), bytes_) );
  }

  template <>
  void memory_t<CUDA>::copyTo(void *dest,
                              const size_t bytes,
                              const size_t offset){
    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CUDA_CHECK("Memory: Copy To",
                    cuMemcpyDtoH(dest, *((CUdeviceptr*) handle) + offset, bytes_) );
  }

  template <>
  void memory_t<CUDA>::copyTo(memory_v *dest,
                              const size_t bytes,
                              const size_t offset){
    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CUDA_CHECK("Memory: Copy To",
                    cuMemcpyDtoD(*((CUdeviceptr*) dest->handle), *((CUdeviceptr*) handle) + offset, bytes_) );
  }

  template <>
  void memory_t<CUDA>::asyncCopyFrom(const void *source,
                                     const size_t bytes,
                                     const size_t offset){
    const CUstream &stream = *((CUstream*) dev->currentStream);

    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CUDA_CHECK("Memory: Asynchronous Copy From",
                    cuMemcpyHtoDAsync(*((CUdeviceptr*) handle) + offset, source, bytes_, stream) );
  }

  template <>
  void memory_t<CUDA>::asyncCopyFrom(const memory_v *source,
                                     const size_t bytes,
                                     const size_t offset){
    const CUstream &stream = *((CUstream*) dev->currentStream);

    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CUDA_CHECK("Memory: Asynchronous Copy From",
                    cuMemcpyDtoDAsync(*((CUdeviceptr*) handle) + offset, *((CUdeviceptr*) source->handle), bytes_, stream) );
  }

  template <>
  void memory_t<CUDA>::asyncCopyTo(void *dest,
                                   const size_t bytes,
                                   const size_t offset){
    const CUstream &stream = *((CUstream*) dev->currentStream);

    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CUDA_CHECK("Memory: Asynchronous Copy To",
                    cuMemcpyDtoHAsync(dest, *((CUdeviceptr*) handle) + offset, bytes_, stream) );
  }

  template <>
  void memory_t<CUDA>::asyncCopyTo(memory_v *dest,
                                   const size_t bytes,
                                   const size_t offset){
    const CUstream &stream = *((CUstream*) dev->currentStream);

    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CUDA_CHECK("Memory: Asynchronous Copy To",
                    cuMemcpyDtoDAsync(*((CUdeviceptr*) dest->handle), *((CUdeviceptr*) handle) + offset, bytes_, stream) );
  }

  template <>
  void memory_t<CUDA>::free(){
    cuMemFree(*((CUdeviceptr*) handle));
    ::free(handle);
  }
  //==================================


  //---[ Device ]---------------------
  template <>
  device_t<CUDA>::device_t() :
    data(NULL),
    memoryUsed(0) {}

  template <>
  device_t<CUDA>::device_t(int platform, int device) :
    data(NULL),
    memoryUsed(0) {}

  template <>
  device_t<CUDA>::device_t(const device_t<CUDA> &d){
    data       = d.data;
    memoryUsed = d.memoryUsed;
  }

  template <>
  device_t<CUDA>& device_t<CUDA>::operator = (const device_t<CUDA> &d){
    data       = d.data;
    memoryUsed = d.memoryUsed;

    return *this;
  }

  template <>
  void device_t<CUDA>::setup(const int platform, const int device){
    static bool cudaIsNotInitialized = true;

    if(cudaIsNotInitialized){
      cuInit(0);
      cudaIsNotInitialized = false;
    }

    data = ::_mm_malloc(sizeof(CUDADeviceData_t), OCCA_MEM_ALIGN);

    OCCA_EXTRACT_DATA(CUDA, Device);

    OCCA_CUDA_CHECK("Device: Creating Device",
                    cuDeviceGet(&data_.device, device));

    OCCA_CUDA_CHECK("Device: Creating Context",
                    cuCtxCreate(&data_.context, CU_CTX_SCHED_AUTO, data_.device));
  }

  template <>
  stream device_t<CUDA>::genStream(){
    OCCA_EXTRACT_DATA(CUDA, Device);

    CUstream *retStream = (CUstream*) ::_mm_malloc(sizeof(CUstream), OCCA_MEM_ALIGN);

    OCCA_CUDA_CHECK("Device: genStream",
                    cuStreamCreate(retStream, CU_STREAM_DEFAULT));

    return retStream;
  }

  template <>
  void device_t<CUDA>::freeStream(stream s){
    OCCA_CUDA_CHECK("Device: freeStream",
                    cuStreamDestroy( *((CUstream*) s) ));
    ::free(s);
  }

  template <>
  kernel_v device_t<CUDA>::buildKernelFromSource(const std::string &filename,
                                                 const std::string &functionName,
                                                 const kernelInfo &info_){
    OCCA_EXTRACT_DATA(CUDA, Device);

    kernel_t<CUDA> k;

    k.dev  = dev;
    k.data = ::_mm_malloc(sizeof(CUDAKernelData_t), OCCA_MEM_ALIGN);

    CUDAKernelData_t &kData_ = *((CUDAKernelData_t*) k.data);

    kData_.device  = data_.device;
    kData_.context = data_.context;

    k.buildFromSource(filename, functionName, info_);
    return k;
  }

  template <>
  kernel_v device_t<CUDA>::buildKernelFromBinary(const std::string &filename,
                                                 const std::string &functionName){
    OCCA_EXTRACT_DATA(CUDA, Device);

    kernel_t<CUDA> k;

    k.dev  = dev;
    k.data = ::_mm_malloc(sizeof(CUDAKernelData_t), OCCA_MEM_ALIGN);

    CUDAKernelData_t &kData_ = *((CUDAKernelData_t*) k.data);

    kData_.device  = data_.device;
    kData_.context = data_.context;

    k.buildFromBinary(filename, functionName);
    return k;
  }

  template <>
  memory_v device_t<CUDA>::malloc(const size_t bytes){
    OCCA_EXTRACT_DATA(CUDA, Device);

    memory_t<CUDA> mem;

    mem.dev    = dev;
    mem.handle = ::_mm_malloc(sizeof(CUdeviceptr), OCCA_MEM_ALIGN);
    mem.size   = bytes;

    OCCA_CUDA_CHECK("Device: malloc",
                    cuMemAlloc((CUdeviceptr*) mem.handle, bytes));

    return mem;
  }

  template <>
  int device_t<CUDA>::simdWidth(){
    if(simdWidth_)
      return simdWidth_;

    OCCA_EXTRACT_DATA(CUDA, Device);

    OCCA_CUDA_CHECK("Device: Get Warp Size",
                    cuDeviceGetAttribute(&simdWidth_,
                                         CU_DEVICE_ATTRIBUTE_WARP_SIZE,
                                         data_.device) );

    return simdWidth_;
  }
  //==================================
};

#endif
