#ifndef __DUMMYARG
#define __DUMMYARG

class occaMemory;

#define DUMMY_ARG_CONSTRUCTOR(TYPE)              \
  dummyArg(const TYPE &arg){                     \
    clpt.TYPE##_  = arg;                         \
    cupt.TYPE##_  = arg;                         \
    cpupt.TYPE##_ = arg;                         \
                                                 \
    clsz  = sizeof(TYPE);                        \
    cusz  = sizeof(TYPE);                        \
    cpusz = sizeof(TYPE);                        \
                                                 \
    isPointer = false;                           \
  }

class dummyArg {
public:
  union dummyArg_t {
    int int_;
    char char_;
    float float_;
    double double_;
    size_t size_t_;
    void * void_;
  } clpt, cupt, cpupt;

  int clsz, cusz, cpusz;
  bool isPointer;

  inline dummyArg(){
    clpt.size_t_  = 0;
    cupt.size_t_  = 0;
    cpupt.size_t_ = 0;

    clsz = cusz = cpusz = 0;

    isPointer = false;
  }

  DUMMY_ARG_CONSTRUCTOR(int);
  DUMMY_ARG_CONSTRUCTOR(char);
  DUMMY_ARG_CONSTRUCTOR(float);
  DUMMY_ARG_CONSTRUCTOR(double);

  dummyArg(const occaMemory &ocmem);
};

#endif
