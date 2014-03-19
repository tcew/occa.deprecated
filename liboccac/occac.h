#ifndef OCCAC_H
#define OCCAC_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define OCCAC_MAX_NUM_ARGS 20

typedef void* occac;

struct occacKernelStruct {
  void*       kernel;
  int         argc;
  void*       args[OCCAC_MAX_NUM_ARGS];
  size_t      argssz[OCCAC_MAX_NUM_ARGS];
};

typedef struct occacKernelStruct * occacKernel;

typedef void* occacMemory;

occac occacNew();

void occacSetup(occac c, const char *threadModel, int platform, int device);

occacKernel occacBuildKernel(occac c, const char *filename,
    const char *kernelname, const char *defFile, const char *flags);

void occacSetThreadArray(occacKernel kernel, size_t *global,
    size_t *local, int dim);

occacMemory occacCreateBuffer(occac c, size_t sz, void *buf);

void occacSetKernelArgBuf(occacKernel kernel,
                          int idx,
                          occacMemory mem);

void occacSetKernelArgRaw(occacKernel kernel,
                       int idx,
                       size_t sz,
                       const void * val);



void occacQueueKernel(occacKernel kernel);

void occacToHost(occacMemory d, void* h);

void occacDestroyBuffer(occac c, occacMemory mem);
void occacDestroyKernel(occacKernel k);
void occacDestroy(occac c);

#ifdef __cplusplus
}
#endif

#endif
