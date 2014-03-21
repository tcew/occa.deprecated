#ifndef OCCA_OPENMP_DEFINES_HEADER
#define OCCA_OPENMP_DEFINES_HEADER

#include <cmath>

#if OCCA_OPENMP_ENABLED
#  include "omp.h"
#endif

//---[ Defines ]----------------------------------
#define OCCA_MAX_THREADS 512
#define OCCA_MEM_ALIGN   16

typedef struct occaArgs_t_ { int data[12]; } occaArgs_t;
//================================================


//---[ Loop Info ]--------------------------------
#define occaOuterDim2 occaArgs.data[0]
#define occaOuterId2  occaArgs.data[1]

#define occaOuterDim1 occaArgs.data[2]
#define occaOuterId1  occaArgs.data[3]

#define occaOuterDim0 occaArgs.data[4]
#define occaOuterId0  occaArgs.data[5]
// - - - - - - - - - - - - - - - - - - - - - - - -
#define occaInnerDim2 occaArgs.data[6]
#define occaInnerId2  occaArgs.data[7]

#define occaInnerDim1 occaArgs.data[8]
#define occaInnerId1  occaArgs.data[9]

#define occaInnerDim0 occaArgs.data[10]
#define occaInnerId0  occaArgs.data[11]
// - - - - - - - - - - - - - - - - - - - - - - - -
#define occaGlobalId2 (occaOuterId2*occaInnerDim2 + occaInnerId2)
#define occaGlobalId1 (occaOuterId1*occaInnerDim1 + occaInnerId1)
#define occaGlobalId0 (occaOuterId0*occaInnerDim0 + occaInnerId0)
//================================================


//---[ Loops ]------------------------------------
#define occaOuterFor2 for(occaOuterId2 = 0; occaOuterId2 < occaOuterDim2; ++occaOuterId2)
#define occaOuterFor1 for(occaOuterId1 = 0; occaOuterId1 < occaOuterDim1; ++occaOuterId1)

#if OCCA_OPENMP_ENABLED
#  define occaOuterFor0                                                   \
  _Pragma("omp parallel for firstprivate(occaOuterId0, occaInnerId0, occaInnerId1, occaInnerId2)") \
  for(occaOuterId0 = 0; occaOuterId0 < occaOuterDim0; ++occaOuterId0)
#else
#  define occaOuterFor0                                         \
  for(occaOuterId0 = 0; occaOuterId0 < occaOuterDim0; ++occaOuterId0)
#endif
// - - - - - - - - - - - - - - - - - - - - - - - -
#define occaInnerFor2 for(occaInnerId2 = 0; occaInnerId2 < occaInnerDim2; ++occaInnerId2)
#define occaInnerFor1 for(occaInnerId1 = 0; occaInnerId1 < occaInnerDim1; ++occaInnerId1)
#define occaInnerFor0 for(occaInnerId0 = 0; occaInnerId0 < occaInnerDim0; ++occaInnerId0)
//================================================


//---[ Standard Functions ]-----------------------
#define occaLocalBarrier()
#define occaGlobalBarrier()
// - - - - - - - - - - - - - - - - - - - - - - - -
#define occaContinue continue
//================================================


//---[ Attributes ]-------------------------------
#define occaShared
#define occaPointer
#define occaVariable &
#define occaRestrict __restrict__
#define occaVolatile volatile
#define occaAligned  __attribute__ ((aligned (OCCA_MEM_ALIGN)))
// - - - - - - - - - - - - - - - - - - - - - - - -
#define occaConst    const
#define occaConstant const
//================================================


//---[ Kernel Info ]------------------------------
#define occaKernelInfoArg   occaArgs_t &occaArgs
#define occaFunctionInfoArg occaArgs_t &occaArgs
#define occaFunctionInfo              occaArgs
// - - - - - - - - - - - - - - - - - - - - - - - -
#define occaKernel         extern "C"
#define occaFunction
#define occaDeviceFunction
//================================================


//---[ Register ]---------------------------------
template <class TM, const int SIZE>
class occaRegister_t {
private:

public:
  const occaArgs_t &occaArgs;

  TM data[OCCA_MAX_THREADS][SIZE] occaAligned;

  occaRegister_t(occaArgs_t &occaArgs_) :
    occaArgs(occaArgs_) {}

  ~occaRegister_t(){}

#define OCCA_REGISTER_ID                                        \
  (occaInnerId2*occaInnerDim1 + occaInnerId1)*occaInnerDim0 + occaInnerId0

  inline TM& operator [] (const int n){
    return data[OCCA_REGISTER_ID][n];
  }

  inline operator TM(){
    return data[OCCA_REGISTER_ID][0];
  }

  inline occaRegister_t& operator = (const occaRegister_t &r) {
    const int id = OCCA_REGISTER_ID;
    data[id][0] = r.data[id][0];
  }

  inline occaRegister_t<TM,SIZE> & operator = (const TM &t){
    data[OCCA_REGISTER_ID][0] = t;
    return *this;
  }

  inline occaRegister_t<TM,SIZE> & operator += (const TM &t){
    data[OCCA_REGISTER_ID][0] += t;
    return *this;
  }

  inline occaRegister_t<TM,SIZE> & operator -= (const TM &t){
    data[OCCA_REGISTER_ID][0] -= t;
    return *this;
  }

  inline occaRegister_t<TM,SIZE> & operator /= (const TM &t){
    data[OCCA_REGISTER_ID][0] /= t;
    return *this;
  }

  inline occaRegister_t<TM,SIZE> & operator *= (const TM &t){
    data[OCCA_REGISTER_ID][0] *= t;
    return *this;
  }
};

#define occaRegisterArray( TYPE , NAME , SIZE )   \
  occaRegister_t<TYPE,SIZE> NAME(occaArgs);

#define occaRegister( TYPE , NAME )               \
  occaRegister_t<TYPE,1> NAME(occaArgs);
//================================================

#endif
