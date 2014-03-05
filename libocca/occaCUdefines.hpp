#define occaCPU 0
#define occaGPU 1

#define occaOpenMP 0
#define occaOpenCL 0
#define occaCUDA   1

#define occaInnerDim0  (blockDim.x)
#define occaInnerDim1  (blockDim.y)
#define occaInnerDim2  (blockDim.z)

#define occaOuterDim0  (gridDim.x)
#define occaOuterDim1  (gridDim.y)

#define occaInnerFor

#define occaInnerFor0
#define occaInnerFor1
#define occaInnerFor2

#define occaOuterFor0
#define occaOuterFor1

#define occaGlobalFor0
#define occaGlobalFor1
#define occaGlobalFor2

#define occaGlobalId0 (threadIdx.x+blockIdx.x*blockDim.x)
#define occaGlobalId1 (threadIdx.y+blockIdx.y*blockDim.y)
#define occaGlobalId2 (threadIdx.z)

#define occaGlobalDim0 (occaInnerDim0*occaOuterDim0)
#define occaGlobalDim1 (occaInnerDim1*occaOuterDim1)
#define occaGlobalDim2 (occaInnerDim2)

#define occaInnerId0  (threadIdx.x)
#define occaInnerId1  (threadIdx.y)
#define occaInnerId2  (threadIdx.z)

#define occaOuterId0  (blockIdx.x)
#define occaOuterId1  (blockIdx.y)

#define occaShared  __shared__

#define occaFunctionShared

#define occaPointer
#define occaVariable
#define occaRestrict __restrict__
#define occaVolatile __volatile__
#define occaConst    const
#define occaConstant __constant__
#define occaAligned

#if 1

#define occaKernel extern "C" __global__
#define occaKernelInfoArg int *dims

#define occaFunctionInfoArg int dummy
#define occaFunctionInfo 1

#define occaFunction __device__
#define occaDeviceFunction __device__

#else
#define occaKernel(KERNEL, ...) extern "C" __global__ void KERNEL(__VA_ARGS__)

#define occaFunction(FUNCTION, ...) __device__ void FUNCTION(__VA_ARGS__)

#define occaFunctionCall(FUNCTION, ...) FUNCTION(__VA_ARGS__)
#endif

#define occaLocalMemFence
#define occaGlobalMemFence
#define occaBarrier(foo) __syncthreads();

#define occaPrivateArray(type, name, n) type name[n]
#define occaPrivate(type, name) type name

//#define occaPrivate(type) type


#define occaUnroll  _Pragma("unroll 16")

#define occaInnerReturn {return;}

#define occaAtomicAdd(p, val) atomicAdd(p,val)
#define occaAtomicSub(p, val) atomicSub(p,val)
