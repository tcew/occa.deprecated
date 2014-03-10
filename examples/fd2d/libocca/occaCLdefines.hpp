//#pragma OPENCL EXTENSION cl_khr_fp64: enable

#pragma OPENCL EXTENSION cl_khr_fp64: enable

#define occaCPU 0
#define occaGPU 1

#define occaOpenMP 0
#define occaOpenCL 1
#define occaCUDA   0

#define occaInnerDim0 (const int) (get_local_size(0))
#define occaInnerDim1 (const int) (get_local_size(1))
#define occaInnerDim2 (const int) (get_local_size(2))

#define occaOuterDim0 (const int) (get_num_groups(0))
#define occaOuterDim1 (const int) (get_num_groups(1))

#define occaGlobalDim0 (get_global_size(0))
#define occaGlobalDim1 (get_global_size(1))
#define occaGlobalDim2 (get_global_size(2))

#define occaInnerFor

#define occaInnerFor0
#define occaInnerFor1
#define occaInnerFor2

#define occaOuterFor0
#define occaOuterFor1

#define occaGlobalFor0
#define occaGlobalFor1
#define occaGlobalFor2

#define occaInnerId0 get_local_id(0)
#define occaInnerId1 get_local_id(1)
#define occaInnerId2 get_local_id(2)

#define occaOuterId0 get_group_id(0)
#define occaOuterId1 get_group_id(1)

#define occaGlobalId0 get_global_id(0)
#define occaGlobalId1 get_global_id(1)
#define occaGlobalId2 get_global_id(2)

#define occaLocalMemFence CLK_LOCAL_MEM_FENCE
#define occaGlobalMemFence CLK_GLOBAL_MEM_FENCE
#define occaBarrier(foo) barrier(foo)

#define occaShared  __local

// #define occaPointer(type) __global type
#define occaPointer __global

#define occaConstant __constant

//#define occaVariable(type) type
#define occaVariable

#define occaRestrict restrict

#define occaVolatile volatile

#define occaConst    const

#define occaAligned

#if 1
#define occaKernelInfoArg  __global int *dims
#define occaFunctionInfoArg  int _dummy
#define occaFunctionInfo  999

#define occaKernel __kernel
#define occaFunction

#else
#define occaKernel(KERNEL, ...) __kernel void KERNEL(__VA_ARGS__)

#define occaFunction(FUNCTION, ...) void FUNCTION(__VA_ARGS__)

#define occaFunctionCall(FUNCTION, ...) FUNCTION(__VA_ARGS__)
#endif
#define occaDeviceFunction

#define occaFunctionShared __local

#define occaPrivateArray(type, name, n) type name[n]
#define occaPrivate(type, name) type name
//#define occaPrivate(type) type


#define occaUnroll  _Pragma("unroll 16")

#define occaInnerReturn {return;}

#define occaAtomicAdd(p, val) atomic_add(p,val)
#define occaAtomicSub(p, val) atomic_sub(p,val)
