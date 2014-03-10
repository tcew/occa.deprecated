#include "stdlib.h"

#define occaInnerDim0 ((const int) ( dims[0] ))
#define occaInnerDim1 ((const int) ( dims[1] ))
#define occaInnerDim2 ((const int) ( dims[2] ))

#define occaOuterDim0 ((const int) ( dims[3] ))
#define occaOuterDim1 ((const int) ( dims[4] ))

#define occaInnerFor0(n) int n; for(n=0;n<occaInnerDim0;++n)
#define occaInnerFor1(n) int n; for(n=0;n<occaInnerDim1;++n)
#define occaInnerFor2(n) int n; for(n=0;n<occaInnerDim2;++n)

#define occaOuterFor0(n) omp_set_num_threads(outerDim0); _Pragma("omp parallel"); int n; _Pragma("omp for")  for(n=0;n<outerDim0;++n)
#define occaOuterFor1(n) int n; for(n=0;n<occaOuterDim1;++n)

#define occaBarrier

#define occaShared

#define occaPointer(type) type

#define occaVariable(type) type &

#define occaRestrict restrict

#define occaConst    const

#define occaKernel(KERNEL, ...) extern "C" void KERNEL(const size_t *dims, __VA_ARGS__)


#define innerDim0 ((const int) ( dims[0] ))
#define innerDim1 ((const int) ( dims[1] ))
#define innerDim2 ((const int) ( dims[2] ))

#define outerDim0 ((const int) ( dims[3] ))
#define outerDim1 ((const int) ( dims[4] ))

//#define innerFor0(n) int n; for(n=0;n<innerDim0;++n)

#define innerFor0(n) int n; for(n=0;n<innerDim0;++n)
#define innerFor1(n) int n; for(n=0;n<innerDim1;++n)
#define innerFor2(n) int n; for(n=0;n<innerDim2;++n)

#define outerFor0(n) int n; _Pragma("omp_set_num_threads(outerDim0)") _Pragma("omp parallel for") for(n=0;n<outerDim0;++n)
//#define outerFor0(n) int n; for(n=0;n<outerDim0;++n)
#define outerFor1(n) int n; for(n=0;n<outerDim1;++n)

#define barrier

#define shared

#define pointer(type) type

#define variable(type) type &

#define restrict restrict

#define const    const

#define kernel(KERNEL, ...) extern "C" void KERNEL(const size_t *dims, __VA_ARGS__)
