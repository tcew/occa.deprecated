#ifndef __OCCACPUDEFINES
#define __OCCACPUDEFINES

#ifndef OCCA_USING_OMP
#  define OCCA_USING_OMP 1
#endif

#include "stdlib.h"
#include "stdio.h"
#include "math.h"

int occaDims0 = 0, occaDims1 = 0, occaDims2 = 0;

typedef struct foo1 { float  x,y; }     float2;
typedef struct foo4 { float  x,y,z,w; } float4;
typedef struct doo2 { double x,y; }     double2;
typedef struct doo4 { double x,y,z,w; } double4;

#define occaCPU 1
#define occaGPU 0

#define occaOpenMP 1
#define occaOpenCL 0
#define occaCUDA   0

#define occaInnerDim0 (occaDims[0])
#define occaInnerDim1 (occaDims[1])
#define occaInnerDim2 (occaDims[2])

#define occaOuterDim0 (occaDims[3])
#define occaOuterDim1 (occaDims[4])
#define occaOuterDim2 (occaDims[5])

#define occaGlobalDim0 (occaInnerDim0*occaOuterDim0)
#define occaGlobalDim1 (occaInnerDim1*occaOuterDim1)
#define occaGlobalDim2 (occaInnerDim2*occaOuterDim2)

// splitting these loops with openmp is problematic because of the global loop variables
// the global loop variables are used to locate private members.
#define occaInnerFor0 for(occaInnerId0 = 0; occaInnerId0 < occaInnerDim0; ++occaInnerId0)
#define occaInnerFor1 for(occaInnerId1 = 0; occaInnerId1 < occaInnerDim1; ++occaInnerId1)
#define occaInnerFor2 for(occaInnerId2 = 0; occaInnerId2 < occaInnerDim2; ++occaInnerId2)

#define occaInnerFor occaInnerFor2 occaInnerFor1 occaInnerFor0

#if OCCA_USING_OMP
#  define occaOuterFor0                                                 \
  int occaInnerId0, occaInnerId1, occaInnerId2;                         \
  _Pragma("omp parallel for firstprivate(occaInnerId0,occaInnerId1,occaInnerId2,occaDims0,occaDims1,occaDims2)") \
  for(int occaOuterId0 = 0; occaOuterId0 < occaOuterDim0; ++occaOuterId0)
#else
#  define occaOuterFor0                                                 \
  int occaInnerId0, occaInnerId1, occaInnerId2;                         \
  for(int occaOuterId0 = 0; occaOuterId0 < occaOuterDim0; ++occaOuterId0)
#endif


#define occaOuterFor1 for(int occaOuterId1 = 0; occaOuterId1 < occaOuterDim1; ++occaOuterId1)

#define occaGlobalFor0 occaOuterFor0 occaInnerFor0
#define occaGlobalFor1 occaOuterFor1 occaInnerFor1
#define occaGlobalFor2 occaInnerFor2

#define occaGlobalId0  ( occaInnerId0 + occaInnerDim0*occaOuterId0 )
#define occaGlobalId1  ( occaInnerId1 + occaInnerDim1*occaOuterId1 )
#define occaGlobalId2  ( occaInnerId2 )

#define occaBarrier

#define occaShared

#define occaPointer

#define occaVariable &

#define occaRestrict __restrict__

#define occaVolatile

// Not all compilers set __BIGGEST_ALIGNMENT__
#ifndef __BIGGEST_ALIGNMENT__
#define __BIGGEST_ALIGNMENT__ 32
#endif

#define occaAligned __attribute__ ((aligned (__BIGGEST_ALIGNMENT__)))

#define occaConst    const
#define occaConstant

#define occaKernelInfoArg const int * __restrict__ occaDims
#define occaFunctionInfoArg const int *  __restrict__ occaDims, \
    int occaInnerId0,                                           \
    int occaInnerId1,                                           \
    int occaInnerId2

#define occaFunctionInfo occaDims,              \
    occaInnerId0,                               \
    occaInnerId1,                               \
    occaInnerId2

#define occaKernel extern "C"
#define occaFunction

#define occaLocalMemFence
#define occaGlobalMemFence
#define occaBarrier(foo)

#define occaFunctionShared


#define occaDeviceFunction

template <class T, const int S>
class occaPrivateClass {
private:

public:
  const int dim0, dim1, dim2;
  const int &id0, &id1, &id2;

  // Warning hard code (256 max threads)
  T data[512][S] occaAligned;

  occaPrivateClass(const int _dim0, const int _dim1, const int _dim2,
                   const int &_id0, const int &_id1, const int &_id2) :
    dim0(_dim0), dim1(_dim1), dim2(_dim2),
    id0(_id0), id1(_id1), id2(_id2) {
  }

  ~occaPrivateClass(){
  }

  inline int index(){
    const int ind = id2*dim0*dim1 + id1*dim0 + id0;
    return ind;
  }

  inline T& operator [] (int n){
    return data[index()][n];
  }

  inline operator T(){
    return data[index()][0];
  }

  occaPrivateClass& operator = (const occaPrivateClass &pc){
    const int idx = index();
    data[idx][0] = pc.data[idx][0];
  }

  inline occaPrivateClass<T,S> & operator= (const T &a){
    data[index()][0] = a;
    return *this;
  }

  inline occaPrivateClass<T,S> & operator+= (const T &a){
    data[index()][0] += a;
    return *this;
  }

  inline occaPrivateClass<T,S> & operator-= (const T &a){
    data[index()][0] -= a;
    return *this;
  }

  inline occaPrivateClass<T,S> & operator/= (const T &a){
    data[index()][0] /= a;
    return *this;
  }

  inline occaPrivateClass<T,S> & operator*= (const T &a){
    data[index()][0] *= a;
    return *this;
  }
};

#define occaPrivateArray(type, name, sz)                                \
  occaPrivateClass<type,sz> name(occaDims[0], occaDims[1], occaDims[2], occaInnerId0, occaInnerId1, occaInnerId2);

#define occaPrivate(type, name)                                         \
  occaPrivateClass<type,1> name(occaDims[0], occaDims[1], occaDims[2], occaInnerId0, occaInnerId1, occaInnerId2);


#define occaUnroll
//_Pragma("unroll 16")

#define occaInnerReturn {continue;}

template <class T> T occaAtomicAdd(T* p, T val) { T old = *p; *p += val; return old; };
template <class T> T occaAtomicSub(T* p, T val) { T old = *p; *p -= val; return old; };

#endif
