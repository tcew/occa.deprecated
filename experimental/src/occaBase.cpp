#include "occa.hpp"

// Use events for timing!

namespace occa {
  std::string ompCompiler , ompCompilerFlags;
  std::string cudaCompiler, cudaArch, cudaCompilerFlags;

  kernelInfo defaultKernelInfo;

  //---[ Kernel ]---------------------
  kernel::kernel() :
    kHandle(NULL) {}

  kernel::kernel(const kernel &k) :
    kHandle(k.kHandle) {}

  kernel& kernel::operator = (const kernel &k){
    kHandle = k.kHandle;

    return *this;
  }

  std::string kernel::mode(){
    return modeToStr(mode_);
  }

  kernel& kernel::buildFromSource(const std::string &filename,
                                  const std::string &functionName_,
                                  const kernelInfo &info_){
    kHandle->buildFromSource(filename, functionName_, info_);

    return *this;
  }

  kernel& kernel::buildFromBinary(const std::string &filename,
                                  const std::string &functionName_){
    kHandle->buildFromBinary(filename, functionName_);

    return *this;
  }

  void kernel::setWorkingDims(int dims, occa::dim inner, occa::dim outer){
    for(int i = 0; i < 3; ++i){
      inner[i] += (inner[i] ? 0 : 1);
      outer[i] += (outer[i] ? 0 : 1);
    }

    kHandle->dims  = dims;
    kHandle->inner = inner;
    kHandle->outer = outer;
  }

  int kernel::preferredDimSize(){
    return kHandle->preferredDimSize();
  }

  OCCA_KERNEL_OPERATOR_DEFINITIONS;

  double kernel::timeTaken(){
    return kHandle->timeTaken();
  }

  void kernel::free(){
    kHandle->free();
    delete kHandle;
  }
  //==================================


  //---[ Memory ]---------------------
  memory::memory() :
    mHandle(NULL) {}

  memory::memory(const memory &m) :
    mHandle(m.mHandle) {}

  memory& memory::operator = (const memory &m){
    mHandle = m.mHandle;

    return *this;
  }

  std::string memory::mode(){
    return modeToStr(mode_);
  }

  void memory::copyFrom(const void *source,
                        const size_t bytes,
                        const size_t offset){
    mHandle->copyFrom(source, bytes, offset);
  }

  void memory::copyFrom(const memory &source,
                        const size_t bytes,
                        const size_t offset){
    mHandle->copyFrom(source.mHandle, bytes, offset);
  }

  void memory::copyTo(void *dest,
                      const size_t bytes,
                      const size_t offset){
    mHandle->copyTo(dest, bytes, offset);
  }

  void memory::copyTo(memory &dest,
                      const size_t bytes,
                      const size_t offset){
    mHandle->copyTo(dest.mHandle, bytes, offset);
  }

  void memory::asyncCopyFrom(const void *source,
                             const size_t bytes,
                             const size_t offset){
    mHandle->asyncCopyFrom(source, bytes, offset);
  }

  void memory::asyncCopyFrom(const memory &source,
                             const size_t bytes,
                             const size_t offset){
    mHandle->asyncCopyFrom(source.mHandle, bytes, offset);
  }

  void memory::asyncCopyTo(void *dest,
                           const size_t bytes,
                           const size_t offset){
    mHandle->asyncCopyTo(dest, bytes, offset);
  }

  void memory::asyncCopyTo(memory &dest,
                           const size_t bytes,
                           const size_t offset){
    mHandle->asyncCopyTo(dest.mHandle, bytes, offset);
  }

  void memory::swap(memory &m){
    occa::mode mode2 = m.mode_;
    m.mode_        = mode_;
    mode_          = mode2;

    memory_v *mHandle2 = m.mHandle;
    m.mHandle          = mHandle;
    mHandle            = mHandle2;
  }

  void memory::free(){
    mHandle->free();
    delete mHandle;
  }
  //==================================


  //---[ Device ]---------------------
  device::device() :
    dHandle(NULL) {}

  device::device(const device &d) :
    mode_(d.mode_),
    dHandle(d.dHandle) {}

  device& device::operator = (const device &d){
    mode_   = d.mode_;
    dHandle = d.dHandle;

    return *this;
  }

  void device::setup(occa::mode m,
                     int platform, int device){
    ompCompiler       = occa::ompCompiler;
    ompCompilerFlags  = occa::ompCompilerFlags;

    cudaCompiler      = occa::cudaCompiler;
    cudaArch          = occa::cudaArch;
    cudaCompilerFlags = occa::cudaCompilerFlags;

    mode_ = m;

    switch(m){
    case OpenMP: dHandle = new device_t<OpenMP>; break;
#if OCCA_OPENCL_ENABLED
    case OpenCL: dHandle = new device_t<OpenCL>; break;
#endif
#if OCCA_CUDA_ENABLED
    case CUDA:   dHandle = new device_t<CUDA>; break;
#endif
    default:
      OCCA_CHECK(false);
    }

    dHandle->dev = this;
    dHandle->setup(platform, device);

    currentStream = genStream();
  }

  void device::setup(const std::string &m,
                     int platform, int device){
    setup(strToMode(m), platform, device);
  }

  std::string device::mode(){
    return modeToStr(mode_);
  }

  stream device::genStream(){
    streams.push_back( dHandle->genStream() );
    return streams.back();
  }

  stream device::getStream(){
    return currentStream;
  }

  void device::setStream(stream s){
    currentStream = s;
  }

  kernel device::buildKernelFromSource(const std::string &filename,
                                       const std::string &functionName,
                                       const kernelInfo &info_){
    kernel ker;

    switch(mode_){
    case OpenMP: ker.kHandle = new kernel_t<OpenMP>; break;
#if OCCA_OPENCL_ENABLED
    case OpenCL: ker.kHandle = new kernel_t<OpenCL>; break;
#endif
#if OCCA_CUDA_ENABLED
    case CUDA  : ker.kHandle = new kernel_t<CUDA>  ; break;
#endif
    default:
      OCCA_CHECK(false);
    }

    ker.kHandle->dev = this;
    *(ker.kHandle)   = dHandle->buildKernelFromSource(filename, functionName, info_);

    return ker;
  }

  kernel device::buildKernelFromBinary(const std::string &filename,
                                       const std::string &functionName){
    kernel ker;

    switch(mode_){
    case OpenMP: ker.kHandle = new kernel_t<OpenMP>; break;
#if OCCA_OPENCL_ENABLED
    case OpenCL: ker.kHandle = new kernel_t<OpenCL>; break;
#endif
#if OCCA_CUDA_ENABLED
    case CUDA  : ker.kHandle = new kernel_t<CUDA>  ; break;
#endif
    default:
      OCCA_CHECK(false);
    }

    ker.kHandle->dev = this;
    (*ker.kHandle)   = dHandle->buildKernelFromBinary(filename, functionName);

    return ker;
  }

  memory device::malloc(const size_t bytes){
    memory mem;

    switch(mode_){
    case OpenMP: mem.mHandle = new memory_t<OpenMP>; break;
#if OCCA_OPENCL_ENABLED
    case OpenCL: mem.mHandle = new memory_t<OpenCL>; break;
#endif
#if OCCA_CUDA_ENABLED
    case CUDA  : mem.mHandle = new memory_t<CUDA>  ; break;
#endif
    default:
      OCCA_CHECK(false);
    }

    mem.mHandle->dev = this;
    (*mem.mHandle)   = dHandle->malloc(bytes);

    return mem;
  }

  void device::free(){
    const int streamCount = streams.size();

    for(int i = 0; i < streamCount; ++i)
      dHandle->freeStream(streams[i]);
  }

  int device::simdWidth(){
    return dHandle->simdWidth();
  }
  //==================================
};
