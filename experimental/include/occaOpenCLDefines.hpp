#ifndef OCCA_OPENCL_DEFINES_HEADER
#define OCCA_OPENCL_DEFINES_HEADER

//---[ Defines ]----------------------------------
//================================================


//---[ Loop Info ]--------------------------------
#define occaOuterDim2 (get_num_groups(2))
#define occaOuterId2  (get_group_id(2))

#define occaOuterDim1 (get_num_groups(1))
#define occaOuterId1  (get_group_id(1))

#define occaOuterDim0 (get_num_groups(0))
#define occaOuterId0  (get_group_id(1))
// - - - - - - - - - - - - - - - - - - - - - - - -
#define occaInnerDim2 (get_local_size(2))
#define occaInnerId2  (get_local_id(2))

#define occaInnerDim1 (get_local_size(1))
#define occaInnerId1  (get_local_id(1))

#define occaInnerDim0 (get_local_size(0))
#define occaInnerId0  (get_local_id(0))
// - - - - - - - - - - - - - - - - - - - - - - - -
#define occaGlobalId2 (get_global_id(2))
#define occaGlobalId1 (get_global_id(1))
#define occaGlobalId0 (get_global_id(0))
//================================================


//---[ Loops ]------------------------------------
#define occaOuterFor2
#define occaOuterFor1
#define occaOuterFor0
// - - - - - - - - - - - - - - - - - - - - - - - -
#define occaInnerFor2
#define occaInnerFor1
#define occaInnerFor0
//================================================


//---[ Standard Functions ]-----------------------
#define occaLocalBarrier()  barrier(CLK_LOCAL_MEM_FENCE)
#define occaGlobalBarrier() barrier(CLK_GLOBAL_MEM_FENCE)
// - - - - - - - - - - - - - - - - - - - - - - - -
#define occaContinue return
//================================================


//---[ Attributes ]-------------------------------
#define occaShared   __local
#define occaPointer  __global
#define occaVariable
#define occaRestrict restrict
#define occaVolatile volatile
#define occaAligned
// - - - - - - - - - - - - - - - - - - - - - - - -
#define occaConst    const
#define occaConstant __constant
//================================================


//---[ Kernel Info ]------------------------------
#define occaKernelInfoArg   int occaKernelInfoArg_
#define occaFunctionInfoArg int occaKernelInfoArg_
#define occaFunctionInfo        occaKernelInfoArg_
// - - - - - - - - - - - - - - - - - - - - - - - -
#define occaKernel         __kernel
#define occaFunction
#define occaDeviceFunction
//================================================


//---[ Register ]---------------------------------
#define occaRegisterArray( TYPE , NAME , SIZE ) TYPE NAME[SIZE]
#define occaRegister( TYPE , NAME )             TYPE NAME
//================================================

#endif
