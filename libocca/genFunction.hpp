#ifndef __GENFUNCTION
#define __GENFUNCTION 1

#include "occaPods.hpp"

#ifdef        OS_LINUX
  typedef timespec occaTime;
#elif defined OS_OSX
  typedef uint64_t occaTime;
#elif defined OS_WINDOWS
#  error "Not tested on Windows OS yet."
#endif

class genFunction {
public:

  int  dim;
  size_t local[3];
  size_t global[3];

  genFunction(){
  }

  virtual void setThreadArray(size_t *in_global, size_t *in_local, int in_dim) = 0;

  GEN_KERNEL_OPERATORS;

  virtual void enqueue(int argc, void* args[], size_t argssz[]) = 0;
};
#endif
