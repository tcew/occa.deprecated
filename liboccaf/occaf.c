#include "occaf_f2c.h"
#include "occac.h"
#include "occafFstring.h"

#ifdef OCCA_DEBUG
#include <stdio.h>
#endif
#include <stdint.h>
#include <inttypes.h>

void OCCAF_OCCAFSETNULL_C(void ** ptr)
{
#ifdef OCCA_DEBUG
  printf("begin occafSetNull_c\n");
#endif
  *ptr = NULL;
#ifdef OCCA_DEBUG
  printf("end   occafSetNull_c\n");
#endif
}

void OCCAF_OCCAFNEW_C(occac * state)
{
#ifdef OCCA_DEBUG
  printf("begin occafNew_c\n");
#endif
  *state = occacNew();
#ifdef OCCA_DEBUG
  printf("end   occafNew_c\n");
#endif
}


void OCCAF_OCCAFSETUP_C(occac * c, OCCAF_CHAR threadModel OCCAF_MIXED_LEN(len),
    int * platform, int * device OCCAF_END_LEN(len))
{
  char * t;
  OCCAF_FIXCHAR(threadModel,len,t);
#ifdef OCCA_DEBUG
  printf("begin occafSetup_c (%s, %d, %d)\n", t, *platform, *device);
#endif
  occacSetup(*c, t, *platform, *device);
#ifdef OCCA_DEBUG
  printf("end   occafSetup_c\n");
#endif
  OCCAF_FREECHAR(threadModel,t);
}

void OCCAF_OCCAFBUILDKERNEL_C(occacKernel * k, occac * c,
    OCCAF_CHAR filename   OCCAF_MIXED_LEN(filen),
    OCCAF_CHAR kernelname OCCAF_MIXED_LEN(krlen),
    OCCAF_CHAR defFile    OCCAF_MIXED_LEN(dflen),
    OCCAF_CHAR flags      OCCAF_MIXED_LEN(fllen)
    OCCAF_END_LEN(filen)
    OCCAF_END_LEN(krlen)
    OCCAF_END_LEN(dflen)
    OCCAF_END_LEN(fllen)
    )
{
  char *fi, *kr, *df, *fl;
  OCCAF_FIXCHAR(filename,  filen,fi);
  OCCAF_FIXCHAR(kernelname,krlen,kr);
  OCCAF_FIXCHAR(defFile,   dflen,df);
  OCCAF_FIXCHAR(flags,     fllen,fl);
#ifdef OCCA_DEBUG
  printf("begin occafBuildKernel_c(%s,%s,%s,%s)\n",fi,kr,df,fl);
#endif
  *k = occacBuildKernel(*c, fi, kr, df, fl);
#ifdef OCCA_DEBUG
  printf("end   occafBuildKernel_c\n");
#endif
  OCCAF_FREECHAR(filename,  fi);
  OCCAF_FREECHAR(kernelname,kr);
  OCCAF_FREECHAR(defFile,   df);
  OCCAF_FREECHAR(flags,     fl);
}

void OCCAF_OCCAFSETTHREADARRAY_C(occacKernel * kernel, int64_t *global,
    int64_t *local, int *dim)
{
#ifdef OCCA_DEBUG
  printf("begin occafSetThreadedArray_c ([%"PRId64", %"PRId64", %"PRId64"],"
                                       " [%"PRId64", %"PRId64", %"PRId64"], %d)\n",
         global[0], global[1], global[2], local[0], local[1], local[2], *dim);
#endif

  size_t s_global[3] = {global[0], global[1], global[2]};
  size_t s_local[3] = {local[0], local[1], local[2]};

 occacSetThreadArray(*kernel, s_global, s_local, *dim);

#ifdef OCCA_DEBUG
  printf("end   occafSetThreadedArray_c\n");
#endif
}

void OCCAF_OCCAFCREATEBUFFER_NULL_C(occacMemory * mem, occac * c, int64_t * sz)
{
#ifdef OCCA_DEBUG
  printf("begin occafCreateBuffer_NULL_c (%"PRId64")\n", *sz);
#endif
  *mem = occacCreateBuffer(*c, (size_t)*sz, NULL);
#ifdef OCCA_DEBUG
  printf("end   occafCreateBuffer_NULL_c\n");
#endif
}

void OCCAF_OCCAFCREATEBUFFER_C(occacMemory * mem, occac * c, int64_t * sz, void **buf)
{
#ifdef OCCA_DEBUG
  printf("begin occafCreateBuffer_c (%"PRId64",%p)\n", *sz, *buf);
#endif
  *mem = occacCreateBuffer(*c, (size_t)*sz, *buf);
#ifdef OCCA_DEBUG
  printf("end   occafCreateBuffer_c\n");
#endif
}

void OCCAF_OCCAFSETKERNELARG_MEM_C(occacKernel *kernel, int *idx, occacMemory *arg)
{
  occacSetKernelArgBuf(*kernel, *idx, *arg);
}

void OCCAF_OCCAFSETKERNELARG_INT4_C(occacKernel *kernel, int *idx, int32_t *arg)
{
  occacSetKernelArgRaw(*kernel, *idx, sizeof(int32_t), arg);
}

void OCCAF_OCCAFSETKERNELARG_INT8_C(occacKernel *kernel, int *idx, int64_t *arg)
{
  occacSetKernelArgRaw(*kernel, *idx, sizeof(int64_t), arg);
}

void OCCAF_OCCAFSETKERNELARG_REAL4_C(occacKernel *kernel, int *idx, float *arg)
{
  occacSetKernelArgRaw(*kernel, *idx, sizeof(float), arg);
}

void OCCAF_OCCAFSETKERNELARG_REAL8_C(occacKernel *kernel, int *idx, double *arg)
{
  occacSetKernelArgRaw(*kernel, *idx, sizeof(double), arg);
}

void OCCAF_OCCAFSETKERNELARG_CHAR_C(occacKernel *kernel, int *idx, char *arg)
{
  occacSetKernelArgRaw(*kernel, *idx, sizeof(char), arg);
}

void OCCAF_OCCAFQUEUEKERNEL_C(occacKernel * kernel)
{
  occacQueueKernel(*kernel);
}

void OCCAF_OCCAFTOHOST_INT4_C(occacMemory *d, void* h)
{
  occacToHost(*d, h);
}

void OCCAF_OCCAFTOHOST_INT8_C(occacMemory *d, void* h)
{
  occacToHost(*d, h);
}

void OCCAF_OCCAFTOHOST_REAL4_C(occacMemory *d, void* h)
{
  occacToHost(*d, h);
}

void OCCAF_OCCAFTOHOST_REAL8_C(occacMemory *d, void* h)
{
  occacToHost(*d, h);
}

void OCCAF_OCCAFTOHOST_CHAR_C(occacMemory *d, void* h)
{
  occacToHost(*d, h);
}

void OCCAF_OCCAFDESTROYBUFFER_C(occac *c, occacMemory *mem)
{
  occacDestroyBuffer(*c, *mem);
}

void OCCAF_OCCAFDESTROYKERNEL_C(occacKernel *k)
{
  occacDestroyKernel(*k);
}

void OCCAF_OCCAFDESTROY_C(occac *c)
{
  occacDestroy(*c);
}
