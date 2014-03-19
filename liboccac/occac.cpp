#include "occac.h"
#include "occa.hpp"
#include <cstdlib>
#include <cstring>



occac occacNew()
{
  occa* dev = new occa;
  return (occac) dev;
}

void occacSetup(occac c, const char *threadModel, int platform, int device)
{
  occa* dev = (occa*) c;
  dev->setup(threadModel, platform, device);
}

occacKernel occacBuildKernel(occac c, const char *filename,
    const char *kernelname, const char *defFile,
    const char *flags)
{
  occa* dev = (occa*) c;

  occacKernel ckernel = new struct occacKernelStruct;

  ckernel->argc = 0;

  for(size_t i = 0; i < OCCAC_MAX_NUM_ARGS; ++i)
    ckernel->args[i] = NULL;

  occaKernel &kernel = dev->buildKernel(filename, kernelname, defFile);

  ckernel->kernel = &kernel;

  return ckernel;
}

void occacSetThreadArray(occacKernel k, size_t *global,
    size_t *local, int dim)
{
  occaKernel* kernel = (occaKernel*) k->kernel;
  kernel->setThreadArray(global, local, dim);
}

occacMemory occacCreateBuffer(occac c, size_t sz, void *buf)
{
  occa* dev = (occa*) c;
  occaMemory &omem = dev->createBuffer(sz, buf);
  return (occacMemory) &omem;
}

void occacSetKernelArgRaw(occacKernel kernel,
                          int idx,
                          size_t sz,
                          const void * val)
{
  if(idx >= OCCAC_MAX_NUM_ARGS)
  {
    std::cerr << "occac wrapper: argument number " << idx << " too big";
    throw 1;
  }

  kernel->argc = std::max(kernel->argc, idx+1);

  if(kernel->args[idx] != NULL && kernel->argssz[idx])
    std::free(kernel->args[idx]);

  kernel->args[idx] = std::malloc(sz);

  if(kernel->args[idx] == NULL)
  {
    std::cerr << "occac wrapper: argument allocation failed ";
    throw 1;
  }

  std::memcpy(kernel->args[idx], val, sz);
  kernel->argssz[idx] = sz;
}

void occacSetKernelArgBuf(occacKernel kernel,
                          int idx,
                          occacMemory mem)
{
  if(idx >= OCCAC_MAX_NUM_ARGS)
  {
    std::cerr << "occac wrapper: argument number " << idx << " too big";
    throw 1;
  }

  kernel->argc = std::max(kernel->argc, idx+1);

  if(kernel->args[idx] != NULL && kernel->argssz[idx])
    std::free(kernel->args[idx]);

  kernel->args[idx] = std::malloc(sizeof(occacMemory));

  if(kernel->args[idx] == NULL)
  {
    std::cerr << "occac wrapper: argument allocation failed ";
    throw 1;
  }

  kernel->args[idx] = mem;
  kernel->argssz[idx] = 0;
}

void occacQueueKernel(occacKernel kernel)
{
  occaKernel* ok = (occaKernel*) kernel->kernel;
  ok->enqueue(kernel->argc, kernel->args, kernel->argssz);
}

void occacToHost(occacMemory d, void* h)
{
  occaMemory* od = (occaMemory*) d;
  od->toHost(h);
}

void occacDestroyBuffer(occac c, occacMemory mem)
{
  occa* dev = (occa*) c;
  occaMemory* omem = (occaMemory*) mem;
  dev->destroyBuffer(*omem);
  delete omem;
}

void occacDestroyKernel(occacKernel k)
{
  occaKernel* ok = (occaKernel*) k->kernel;
  delete ok;

  for(size_t i = 0; i < OCCAC_MAX_NUM_ARGS; ++i)
    if(k->args[i] != NULL && k->argssz[i])
      std::free(k->args[i]);

  delete k;
}

void occacDestroy(occac c)
{
  occa* dev = (occa*) c;
  delete dev;
}
