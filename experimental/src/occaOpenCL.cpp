#if OCCA_OPENCL_ENABLED

#include "occaOpenCL.hpp"

namespace occa {
  //---[ Kernel ]---------------------
  template <>
  kernel_t<OpenCL>::kernel_t(){
    data = NULL;
    dev  = NULL;

    functionName = "";

    dims  = 1;
    inner = occa::dim(1,1,1);
    outer = occa::dim(1,1,1);

    preferredDimSize_ = 0;

    startTime = (void*) new cl_event;
    endTime   = (void*) new cl_event;
  }

  template <>
  kernel_t<OpenCL>::kernel_t(const kernel_t<OpenCL> &k){
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
  kernel_t<OpenCL>& kernel_t<OpenCL>::operator = (const kernel_t<OpenCL> &k){
    data = k.data;
    dev  = k.dev;

    functionName = k.functionName;

    dims  = k.dims;
    inner = k.inner;
    outer = k.outer;

    preferredDimSize_ = k.preferredDimSize_;

    *((cl_event*) startTime) = *((cl_event*) k.startTime);
    *((cl_event*) endTime)   = *((cl_event*) k.endTime);

    return *this;
  }

  template <>
  kernel_t<OpenCL>::~kernel_t(){}

  template <>
  kernel_t<OpenCL>& kernel_t<OpenCL>::buildFromSource(const std::string &filename,
                                                      const std::string &functionName_,
                                                      const kernelInfo &info_){
    OCCA_EXTRACT_DATA(OpenCL, Kernel);

    functionName = functionName_;

    kernelInfo info = info_;
    info.addDefine("OCCA_USING_CPU", 0);
    info.addDefine("OCCA_USING_GPU", 1);

    info.addDefine("OCCA_USING_OPENMP", 0);
    info.addDefine("OCCA_USING_OPENCL", 1);
    info.addDefine("OCCA_USING_CUDA"  , 0);

    info.addOCCAKeywords(occaOpenCLDefines);

    std::stringstream salt;
    salt << "OpenCL"
         << data_.platformDevice.x << '-' << data_.platformDevice.y
         << info.salt();

    std::string cachedBinary = binaryIsCached(filename, salt.str());

    if(fileExists(cachedBinary)){
      std::cout << "Found cached binary of [" << filename << "] in [" << cachedBinary << "]\n";
      return buildFromBinary(cachedBinary, functionName);
    }

    std::string iCachedBinary = createIntermediateSource(filename,
                                                         cachedBinary,
                                                         info);

    cl_int error;

    const char *cFunction = ocl::cReadFile(iCachedBinary);
    const size_t cLength  = ocl::fileSize(iCachedBinary);

    data_.program = clCreateProgramWithSource(data_.context, 1, &cFunction, &cLength, &error);
    OCCA_CL_CHECK("Kernel (" + functionName + ") : Constructing Program", error);

    error = clBuildProgram(data_.program, 1, &data_.deviceID, info.flags.c_str(), NULL, NULL);

    if(error)
      cl::printProgramBuildLog(functionName, data_.program, data_.deviceID);

    OCCA_CL_CHECK("Kernel (" + functionName + ") : Building Program", error);

    cl::saveProgramBinary(data_.program, cachedBinary);

    data_.kernel = clCreateKernel(data_.program, functionName.c_str(), &error);
    OCCA_CL_CHECK("Kernel (" + functionName + "): Creating Kernel", error);

    std::cout << "OpenCL [" << cl::getPlatformVendor(data_.platformID) << "] compiled " << filename;

    delete [] cFunction;

    return *this;
  }

  template <>
  kernel_t<OpenCL>& kernel_t<OpenCL>::buildFromBinary(const std::string &filename,
                                                      const std::string &functionName_){
    OCCA_EXTRACT_DATA(OpenCL, Kernel);

    functionName = functionName_;

    cl_int binaryError, error;

    const unsigned char *cFile = (const unsigned char*) ocl::cReadFile(filename);
    size_t fileSize = ocl::fileSize(filename);

    data_.program = clCreateProgramWithBinary(data_.context,
                                              1, &(data_.deviceID),
                                              &fileSize, &cFile,
                                              &binaryError, &error);
    OCCA_CL_CHECK("Kernel (" + functionName + ") : Constructing Program", binaryError);
    OCCA_CL_CHECK("Kernel (" + functionName + ") : Constructing Program", error);

    error = clBuildProgram(data_.program, 1, &data_.deviceID, NULL, NULL, NULL); // <> Needs flags!

    if(error)
      cl::printProgramBuildLog(functionName, data_.program, data_.deviceID);

    OCCA_CL_CHECK("Kernel (" + functionName + ") : Building Program", error);

    data_.kernel = clCreateKernel(data_.program, functionName.c_str(), &error);
    OCCA_CL_CHECK("Kernel (" + functionName + "): Creating Kernel", error);

    delete [] cFile;

    return *this;
  }

  template <>
  int kernel_t<OpenCL>::preferredDimSize(){
    if(preferredDimSize_)
      return preferredDimSize_;

    OCCA_EXTRACT_DATA(OpenCL, Kernel);

    size_t pds;

    OCCA_CL_CHECK("Kernel: Getting Preferred Dim Size",
                  clGetKernelWorkGroupInfo(data_.kernel,
                                           data_.deviceID,
                                           CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                           sizeof(size_t), &pds, NULL));

    preferredDimSize_ = pds;

    return preferredDimSize_;
  }

  OCCA_OPENCL_KERNEL_OPERATOR_DEFINITIONS;

  template <>
  double kernel_t<OpenCL>::timeTaken(){
    cl_event &startEvent = *((cl_event*) startTime);
    cl_event &endEvent   = *((cl_event*) endTime);

    cl_ulong start, end;

    clGetEventProfilingInfo(startEvent, CL_PROFILING_COMMAND_END,
                            sizeof(cl_ulong), &start,
                            NULL);

    clGetEventProfilingInfo(endEvent, CL_PROFILING_COMMAND_START,
                            sizeof(cl_ulong), &end,
                            NULL);

    return 1.0e-9*(end - start);
  }

  template <>
  void kernel_t<OpenCL>::free(){
  }
  //==================================


  //---[ Memory ]---------------------
  template <>
  memory_t<OpenCL>::memory_t(){
    handle = NULL;
    dev    = NULL;
    size = 0;
  }

  template <>
  memory_t<OpenCL>::memory_t(const memory_t<OpenCL> &m){
    handle = m.handle;
    dev    = m.dev;
    size   = m.size;
  }

  template <>
  memory_t<OpenCL>& memory_t<OpenCL>::operator = (const memory_t<OpenCL> &m){
    handle = m.handle;
    dev    = m.dev;
    size   = m.size;

    return *this;
  }

  template <>
  memory_t<OpenCL>::~memory_t(){}

  template <>
  void memory_t<OpenCL>::copyFrom(const void *source,
                                  const size_t bytes,
                                  const size_t offset){
    cl_command_queue &stream = *((cl_command_queue*) dev->currentStream);

    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CL_CHECK("Memory: Copy From",
                  clEnqueueWriteBuffer(stream, *((cl_mem*) handle),
                                       CL_TRUE,
                                       offset, bytes_, source,
                                       0, NULL, NULL));
  }

  template <>
  void memory_t<OpenCL>::copyFrom(const memory_v *source,
                                  const size_t bytes,
                                  const size_t offset){
    cl_command_queue &stream = *((cl_command_queue*) dev->currentStream);

    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CL_CHECK("Memory: Copy From",
                  clEnqueueCopyBuffer(stream,
                                      *((cl_mem*) source->handle),
                                      *((cl_mem*) handle),
                                      0, offset,// <>
                                      bytes_,
                                      0, NULL, NULL));
  }

  template <>
  void memory_t<OpenCL>::copyTo(void *dest,
                                const size_t bytes,
                                const size_t offset){
    const cl_command_queue &stream = *((cl_command_queue*) dev->currentStream);

    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CL_CHECK("Memory: Copy To",
                  clEnqueueReadBuffer(stream, *((cl_mem*) handle),
                                      CL_TRUE,
                                      offset, bytes_, dest,
                                      0, NULL, NULL));
  }

  template <>
  void memory_t<OpenCL>::copyTo(memory_v *dest,
                                const size_t bytes,
                                const size_t offset){
    const cl_command_queue &stream = *((cl_command_queue*) dev->currentStream);

    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CL_CHECK("Memory: Copy To",
                  clEnqueueCopyBuffer(stream,
                                      *((cl_mem*) handle),
                                      *((cl_mem*) dest->handle),
                                      offset, 0,// <>
                                      bytes_,
                                      0, NULL, NULL));
  }

  template <>
  void memory_t<OpenCL>::asyncCopyFrom(const void *source,
                                       const size_t bytes,
                                       const size_t offset){
    const cl_command_queue &stream = *((cl_command_queue*) dev->currentStream);

    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CL_CHECK("Memory: Asynchronous Copy From",
                  clEnqueueWriteBuffer(stream, *((cl_mem*) handle),
                                       CL_FALSE,
                                       offset, bytes_, source,
                                       0, NULL, NULL));
  }

  template <>
  void memory_t<OpenCL>::asyncCopyFrom(const memory_v *source,
                                       const size_t bytes,
                                       const size_t offset){
    const cl_command_queue &stream = *((cl_command_queue*) dev->currentStream);

    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CL_CHECK("Memory: Asynchronous Copy From",
                  clEnqueueCopyBuffer(stream,
                                      *((cl_mem*) source->handle),
                                      *((cl_mem*) handle),
                                      0, offset,// <>
                                      bytes_,
                                      0, NULL, NULL));
  }

  template <>
  void memory_t<OpenCL>::asyncCopyTo(void *dest,
                                     const size_t bytes,
                                     const size_t offset){
    const cl_command_queue &stream = *((cl_command_queue*) dev->currentStream);

    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CL_CHECK("Memory: Asynchronous Copy To",
                  clEnqueueReadBuffer(stream, *((cl_mem*) handle),
                                      CL_FALSE,
                                      offset, bytes_, dest,
                                      0, NULL, NULL));
  }

  template <>
  void memory_t<OpenCL>::asyncCopyTo(memory_v *dest,
                                     const size_t bytes,
                                     const size_t offset){
    const cl_command_queue &stream = *((cl_command_queue*) dev->currentStream);

    const size_t bytes_ = (bytes == 0) ? size : bytes;

    OCCA_CHECK((bytes_ + offset) <= size);

    OCCA_CL_CHECK("Memory: Asynchronous Copy To",
                  clEnqueueCopyBuffer(stream,
                                      *((cl_mem*) handle),
                                      *((cl_mem*) dest->handle),
                                      offset, 0, // <>
                                      bytes_,
                                      0, NULL, NULL));
  }

  template <>
  void memory_t<OpenCL>::free(){
    clReleaseMemObject(*((cl_mem*) handle));
    ::free(handle);
  }
  //==================================


  //---[ Device ]---------------------
  template <>
  device_t<OpenCL>::device_t() :
    data(NULL),
    memoryUsed(0) {}

  template <>
  device_t<OpenCL>::device_t(int platform, int device) :
    data(NULL),
    memoryUsed(0) {}

  template <>
  device_t<OpenCL>::device_t(const device_t<OpenCL> &d){
    data       = d.data;
    memoryUsed = d.memoryUsed;
  }

  template <>
  device_t<OpenCL>& device_t<OpenCL>::operator = (const device_t<OpenCL> &d){
    data       = d.data;
    memoryUsed = d.memoryUsed;

    return *this;
  }

  template <>
  void device_t<OpenCL>::setup(const int platform, const int device){
    data = ::_mm_malloc(sizeof(OpenCLDeviceData_t), OCCA_MEM_ALIGN);

    OCCA_EXTRACT_DATA(OpenCL, Device);
    cl_int error;

    data_.platformDevice = int2(platform, device);

    data_.platformID = cl::getPlatformID(platform);
    data_.deviceID   = cl::getDeviceID(platform, device);

    data_.context = clCreateContext(NULL, 1, &data_.deviceID, NULL, NULL, &error);
    OCCA_CL_CHECK("Device: Creating Context", error);
  }

  template <>
  stream device_t<OpenCL>::genStream(){
    OCCA_EXTRACT_DATA(OpenCL, Device);
    cl_int error;

    cl_command_queue *retStream = (cl_command_queue*) ::_mm_malloc(sizeof(cl_command_queue), OCCA_MEM_ALIGN);

    *retStream = clCreateCommandQueue(data_.context, data_.deviceID, CL_QUEUE_PROFILING_ENABLE, &error);
    OCCA_CL_CHECK("Device: genStream", error);

    return retStream;
  }

  template <>
  void device_t<OpenCL>::freeStream(stream s){
    OCCA_CL_CHECK("Device: freeStream",
                  clReleaseCommandQueue( *((cl_command_queue*) s) ));
    ::free(s);
  }

  template <>
  kernel_v device_t<OpenCL>::buildKernelFromSource(const std::string &filename,
                                                   const std::string &functionName,
                                                   const kernelInfo &info_){
    OCCA_EXTRACT_DATA(OpenCL, Device);

    kernel_t<OpenCL> k;

    k.dev  = dev;
    k.data = ::_mm_malloc(sizeof(OpenCLKernelData_t), OCCA_MEM_ALIGN);

    OpenCLKernelData_t &kData_ = *((OpenCLKernelData_t*) k.data);

    kData_.platformDevice = data_.platformDevice;

    kData_.platformID = data_.platformID;
    kData_.deviceID   = data_.deviceID;
    kData_.context    = data_.context;

    k.buildFromSource(filename, functionName, info_);
    return k;
  }

  template <>
  kernel_v device_t<OpenCL>::buildKernelFromBinary(const std::string &filename,
                                                   const std::string &functionName){
    OCCA_EXTRACT_DATA(OpenCL, Device);

    kernel_t<OpenCL> k;

    k.dev  = dev;
    k.data = ::_mm_malloc(sizeof(OpenCLKernelData_t), OCCA_MEM_ALIGN);

    OpenCLKernelData_t &kData_ = *((OpenCLKernelData_t*) k.data);

    kData_.platformDevice = data_.platformDevice;

    kData_.platformID = data_.platformID;
    kData_.deviceID   = data_.deviceID;
    kData_.context    = data_.context;

    k.buildFromBinary(filename, functionName);
    return k;
  }

  template <>
  memory_v device_t<OpenCL>::malloc(const size_t bytes){
    OCCA_EXTRACT_DATA(OpenCL, Device);

    memory_t<OpenCL> mem;
    cl_int error;

    mem.dev    = dev;
    mem.handle = ::_mm_malloc(sizeof(cl_mem), OCCA_MEM_ALIGN);
    mem.size   = bytes;

    *((cl_mem*) mem.handle) = clCreateBuffer(data_.context, CL_MEM_READ_WRITE,
                                             bytes, NULL, &error);
    OCCA_CL_CHECK("Device: malloc", error);

    return mem;
  }

  template <>
  int device_t<OpenCL>::simdWidth(){
    if(simdWidth_)
      return simdWidth_;

    OCCA_EXTRACT_DATA(OpenCL, Device);

    if( cl::deviceIsAGPU(data_.deviceID) ){
      std::string deviceVendor = cl::getDeviceVendor(data_.deviceID);

      if(deviceVendor == "NVIDIA")
        simdWidth_ = 32;
      else if(deviceVendor == "AMD")
        simdWidth_ = 64;
      else if(deviceVendor == "Intel" || deviceVendor == "Apple") // <> Need to check for Xeon Phi
        simdWidth_ = OCCA_SIMD_WIDTH;
      else{
        OCCA_CHECK(false);
      }
    }
    else
      simdWidth_ = OCCA_SIMD_WIDTH;

#if 0
    cl_device_type dBuffer;
    bool isGPU;

    const int bSize = 8192;
    char buffer[bSize + 1];
    buffer[bSize] = '\0';

    OCCA_CL_CHECK("Device: DEVICE_TYPE",
                  clGetDeviceInfo(data_.deviceID, CL_DEVICE_TYPE, sizeof(dBuffer), &dBuffer, NULL));

    OCCA_CL_CHECK("Device: DEVICE_VENDOR",
                  clGetDeviceInfo(data_.deviceID, CL_DEVICE_VENDOR, bSize, buffer, NULL));

    if(dBuffer & CL_DEVICE_TYPE_CPU)
      isGPU = false;
    else if(dBuffer & CL_DEVICE_TYPE_GPU)
      isGPU = true;
    else{
      OCCA_CHECK(false);
    }

    if(isGPU){
      std::string vendor = buffer;
      if(vendor.find("NVIDIA") != std::string::npos)
        simdWidth_ = 32;
      else if((vendor.find("AMD")                    != std::string::npos) ||
              (vendor.find("Advanced Micro Devices") != std::string::npos))
        simdWidth_ = 64;
      else if(vendor.find("Intel") != std::string::npos)   // <> Need to check for Xeon Phi
        simdWidth_ = OCCA_SIMD_WIDTH;
      else{
        OCCA_CHECK(false);
      }
    }
    else
      simdWidth_ = OCCA_SIMD_WIDTH;
#endif

    return simdWidth_;
  }
  //==================================
};

#endif
