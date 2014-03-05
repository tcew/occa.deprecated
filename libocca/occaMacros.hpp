#define EMPTY /**/

#define DUMMY_ARG_1(TYPE)                       TYPE arg1
#define DUMMY_ARG_2(TYPE)  DUMMY_ARG_1(TYPE)  , TYPE arg2
#define DUMMY_ARG_3(TYPE)  DUMMY_ARG_2(TYPE)  , TYPE arg3
#define DUMMY_ARG_4(TYPE)  DUMMY_ARG_3(TYPE)  , TYPE arg4
#define DUMMY_ARG_5(TYPE)  DUMMY_ARG_4(TYPE)  , TYPE arg5
#define DUMMY_ARG_6(TYPE)  DUMMY_ARG_5(TYPE)  , TYPE arg6
#define DUMMY_ARG_7(TYPE)  DUMMY_ARG_6(TYPE)  , TYPE arg7
#define DUMMY_ARG_8(TYPE)  DUMMY_ARG_7(TYPE)  , TYPE arg8
#define DUMMY_ARG_9(TYPE)  DUMMY_ARG_8(TYPE)  , TYPE arg9
#define DUMMY_ARG_10(TYPE) DUMMY_ARG_9(TYPE)  , TYPE arg10
#define DUMMY_ARG_11(TYPE) DUMMY_ARG_10(TYPE) , TYPE arg11
#define DUMMY_ARG_12(TYPE) DUMMY_ARG_11(TYPE) , TYPE arg12
#define DUMMY_ARG_13(TYPE) DUMMY_ARG_12(TYPE) , TYPE arg13
#define DUMMY_ARG_14(TYPE) DUMMY_ARG_13(TYPE) , TYPE arg14
#define DUMMY_ARG_15(TYPE) DUMMY_ARG_14(TYPE) , TYPE arg15
#define DUMMY_ARG_16(TYPE) DUMMY_ARG_15(TYPE) , TYPE arg16
#define DUMMY_ARG_17(TYPE) DUMMY_ARG_16(TYPE) , TYPE arg17
#define DUMMY_ARG_18(TYPE) DUMMY_ARG_17(TYPE) , TYPE arg18
#define DUMMY_ARG_19(TYPE) DUMMY_ARG_18(TYPE) , TYPE arg19
#define DUMMY_ARG_20(TYPE) DUMMY_ARG_19(TYPE) , TYPE arg20
#define DUMMY_ARG_21(TYPE) DUMMY_ARG_20(TYPE) , TYPE arg21

#define DUMMY_ARGUMENTS1(N)      DUMMY_ARG_##N(const dummyArg &)
#define DUMMY_ARGUMENTS2(N)      DUMMY_ARG_##N(EMPTY)

#if OCCA_USE_OPENCL==1
#define CL_OCCA_ARGS                                                    \
  CL_CHECK( clSetKernelArg(kernel, 0, sizeof(cldims), &cldims) );

#define CL_SET_KERNEL_ARG_FUNC(N)                                       \
  if(arg##N.isPointer)                                                  \
    CL_CHECK( clSetKernelArg(kernel, N, arg##N.clsz, arg##N.clpt.void_)); \
  else                                                                  \
    CL_CHECK( clSetKernelArg(kernel, N, arg##N.clsz, (void*) &arg##N.clpt));
#endif

#if OCCA_USE_CUDA==1
#define CU_OCCA_ARGS              &cudims
#define CU_SET_KERNEL_ARG_FUNC(N) , (arg##N.isPointer ? arg##N.cupt.void_ : (void*) &arg##N.cupt)
#endif

#if OCCA_USE_CPU==1
#define CPU_OCCA_ARGS              dims
#define CPU_SET_KERNEL_ARG_FUNC(N) , (arg##N.isPointer ? arg##N.cpupt.void_ : (void*) &arg##N.cpupt)
#endif

#define OCCA_ARGS(MODEL)              MODEL##_OCCA_ARGS
#define SET_KERNEL_ARG_FUNC(MODEL, N) MODEL##_SET_KERNEL_ARG_FUNC(N)


#define SET_KERNEL_ARG_1(MODEL)  OCCA_ARGS(MODEL)         SET_KERNEL_ARG_FUNC(MODEL, 1)
#define SET_KERNEL_ARG_2(MODEL)  SET_KERNEL_ARG_1(MODEL)  SET_KERNEL_ARG_FUNC(MODEL, 2)
#define SET_KERNEL_ARG_3(MODEL)  SET_KERNEL_ARG_2(MODEL)  SET_KERNEL_ARG_FUNC(MODEL, 3)
#define SET_KERNEL_ARG_4(MODEL)  SET_KERNEL_ARG_3(MODEL)  SET_KERNEL_ARG_FUNC(MODEL, 4)
#define SET_KERNEL_ARG_5(MODEL)  SET_KERNEL_ARG_4(MODEL)  SET_KERNEL_ARG_FUNC(MODEL, 5)
#define SET_KERNEL_ARG_6(MODEL)  SET_KERNEL_ARG_5(MODEL)  SET_KERNEL_ARG_FUNC(MODEL, 6)
#define SET_KERNEL_ARG_7(MODEL)  SET_KERNEL_ARG_6(MODEL)  SET_KERNEL_ARG_FUNC(MODEL, 7)
#define SET_KERNEL_ARG_8(MODEL)  SET_KERNEL_ARG_7(MODEL)  SET_KERNEL_ARG_FUNC(MODEL, 8)
#define SET_KERNEL_ARG_9(MODEL)  SET_KERNEL_ARG_8(MODEL)  SET_KERNEL_ARG_FUNC(MODEL, 9)
#define SET_KERNEL_ARG_10(MODEL) SET_KERNEL_ARG_9(MODEL)  SET_KERNEL_ARG_FUNC(MODEL, 10)
#define SET_KERNEL_ARG_11(MODEL) SET_KERNEL_ARG_10(MODEL) SET_KERNEL_ARG_FUNC(MODEL, 11)
#define SET_KERNEL_ARG_12(MODEL) SET_KERNEL_ARG_11(MODEL) SET_KERNEL_ARG_FUNC(MODEL, 12)
#define SET_KERNEL_ARG_13(MODEL) SET_KERNEL_ARG_12(MODEL) SET_KERNEL_ARG_FUNC(MODEL, 13)
#define SET_KERNEL_ARG_14(MODEL) SET_KERNEL_ARG_13(MODEL) SET_KERNEL_ARG_FUNC(MODEL, 14)
#define SET_KERNEL_ARG_15(MODEL) SET_KERNEL_ARG_14(MODEL) SET_KERNEL_ARG_FUNC(MODEL, 15)
#define SET_KERNEL_ARG_16(MODEL) SET_KERNEL_ARG_15(MODEL) SET_KERNEL_ARG_FUNC(MODEL, 16)
#define SET_KERNEL_ARG_17(MODEL) SET_KERNEL_ARG_16(MODEL) SET_KERNEL_ARG_FUNC(MODEL, 17)
#define SET_KERNEL_ARG_18(MODEL) SET_KERNEL_ARG_17(MODEL) SET_KERNEL_ARG_FUNC(MODEL, 18)
#define SET_KERNEL_ARG_19(MODEL) SET_KERNEL_ARG_18(MODEL) SET_KERNEL_ARG_FUNC(MODEL, 19)
#define SET_KERNEL_ARG_20(MODEL) SET_KERNEL_ARG_19(MODEL) SET_KERNEL_ARG_FUNC(MODEL, 20)

#define SET_KERNEL_ARG(MODEL, N) SET_KERNEL_ARG_##N(MODEL)

#if OCCA_USE_OPENCL==1
#define OCCA_CLFN(N) if(model & OpenCL) clfn(DUMMY_ARGUMENTS2(N))
#else
#define OCCA_CLFN(N)
#endif

#if OCCA_USE_CUDA==1
#define OCCA_CUFN(N) if(model & CUDA)   cufn(DUMMY_ARGUMENTS2(N))
#else
#define OCCA_CUFN(N)
#endif

#if OCCA_USE_CPU==1
#define OCCA_CPUFN(N) if(model & CPU)   cpufn(DUMMY_ARGUMENTS2(N))
#else
#define OCCA_CPUFN(N)
#endif

#define OCCA_KERNEL_OPERATOR(N)                 \
  void operator() (DUMMY_ARGUMENTS1(N)){        \
    OCCA_CLFN(N);                               \
    OCCA_CUFN(N);                               \
    OCCA_CPUFN(N);                              \
  }

#define GEN_KERNEL_OPERATOR(N)                          \
  virtual void operator()(DUMMY_ARGUMENTS1(N)) = 0;

#if OCCA_USE_OPENCL==1
#define CL_KERNEL_OPERATOR(N)                   \
  void operator() (DUMMY_ARGUMENTS1(N)){        \
    SET_KERNEL_ARG(CL, N);                      \
    run();                                      \
  }
#endif

#if OCCA_USE_CUDA==1
#define CU_KERNEL_OPERATOR(N)                                           \
  void operator() (DUMMY_ARGUMENTS1(N)){                                \
    void *args[] = {SET_KERNEL_ARG(CU, N)};                             \
                                                                        \
    cuEventRecord(ev_start,0);						\
    cuLaunchKernel(kernel,                                              \
                   global[0]/local[0], global[1]/local[1], 1,           \
                   local[0], local[1], local[2],                        \
                   0, 0, args, 0);                                      \
    cuEventRecord(ev_end,0);                                            \
  }
#endif

#if OCCA_USE_CPU==1
#define CPU_KERNEL_OPERATOR(N)                                  \
  void operator() (DUMMY_ARGUMENTS1(N)){                        \
    tic();                                                      \
    functionPointer##N tmpKernel = (functionPointer##N) kernel; \
      tmpKernel(SET_KERNEL_ARG(CPU, N));                        \
      toc();                                                    \
  }
#endif

#define FOR_20(EXPR)                            \
  EXPR(1);                                      \
  EXPR(2);                                      \
  EXPR(3);                                      \
  EXPR(4);                                      \
  EXPR(5);                                      \
  EXPR(6);                                      \
  EXPR(7);                                      \
  EXPR(8);                                      \
  EXPR(9);                                      \
  EXPR(10);                                     \
  EXPR(11);                                     \
  EXPR(12);                                     \
  EXPR(13);                                     \
  EXPR(14);                                     \
  EXPR(15);                                     \
  EXPR(16);                                     \
  EXPR(17);                                     \
  EXPR(18);                                     \
  EXPR(19);                                     \
  EXPR(20);

#define OCCA_KERNEL_OPERATORS FOR_20(OCCA_KERNEL_OPERATOR)
#define GEN_KERNEL_OPERATORS  FOR_20(GEN_KERNEL_OPERATOR)

#if OCCA_USE_OPENCL==1
#define CL_KERNEL_OPERATORS   FOR_20(CL_KERNEL_OPERATOR)
#endif

#if OCCA_USE_CUDA==1
#define CU_KERNEL_OPERATORS   FOR_20(CU_KERNEL_OPERATOR)
#endif

#if OCCA_USE_CPU==1
#define CPU_KERNEL_OPERATORS  FOR_20(CPU_KERNEL_OPERATOR)
#endif

#define VOID_ARG_1        void*,  void*
#define VOID_ARG_2  VOID_ARG_1  , void*
#define VOID_ARG_3  VOID_ARG_2  , void*
#define VOID_ARG_4  VOID_ARG_3  , void*
#define VOID_ARG_5  VOID_ARG_4  , void*
#define VOID_ARG_6  VOID_ARG_5  , void*
#define VOID_ARG_7  VOID_ARG_6  , void*
#define VOID_ARG_8  VOID_ARG_7  , void*
#define VOID_ARG_9  VOID_ARG_8  , void*
#define VOID_ARG_10 VOID_ARG_9  , void*
#define VOID_ARG_11 VOID_ARG_10 , void*
#define VOID_ARG_12 VOID_ARG_11 , void*
#define VOID_ARG_13 VOID_ARG_12 , void*
#define VOID_ARG_14 VOID_ARG_13 , void*
#define VOID_ARG_15 VOID_ARG_14 , void*
#define VOID_ARG_16 VOID_ARG_15 , void*
#define VOID_ARG_17 VOID_ARG_16 , void*
#define VOID_ARG_18 VOID_ARG_17 , void*
#define VOID_ARG_19 VOID_ARG_18 , void*
#define VOID_ARG_20 VOID_ARG_19 , void*

#define FUNCTION_POINTER(N)                             \
  typedef void (*functionPointer##N)( VOID_ARG_##N )

#define FUNCTION_POINTER_TYPEDEF                \
  typedef void (*voidFunction)();               \
  FUNCTION_POINTER(1);                          \
  FUNCTION_POINTER(2);                          \
  FUNCTION_POINTER(3);                          \
  FUNCTION_POINTER(4);                          \
  FUNCTION_POINTER(5);                          \
  FUNCTION_POINTER(6);                          \
  FUNCTION_POINTER(7);                          \
  FUNCTION_POINTER(8);                          \
  FUNCTION_POINTER(9);                          \
  FUNCTION_POINTER(10);                         \
  FUNCTION_POINTER(11);                         \
  FUNCTION_POINTER(12);                         \
  FUNCTION_POINTER(13);                         \
  FUNCTION_POINTER(14);                         \
  FUNCTION_POINTER(15);                         \
  FUNCTION_POINTER(16);                         \
  FUNCTION_POINTER(17);                         \
  FUNCTION_POINTER(18);                         \
  FUNCTION_POINTER(19);                         \
  FUNCTION_POINTER(20);
