#include <stdlib.h>
#include <stdio.h>
#include "occac.h"


int main(int argc, char **argv){

  int platform = 0, device = 0;

  occac dev = occacNew();

  /* grab platform and device */
  occacSetup(dev, "CPU", platform, device);

  /* build as cl kernel */
  occacKernel simpleKernel =
    occacBuildKernel(dev, "simple.occa", "simple", "", "");

  /* set thread array for kernel */
  int dim = 1;
  size_t outer[3] = {100*128,1,1};
  size_t inner[3] = {128,1,1};
  occacSetThreadArray(simpleKernel, outer, inner, dim);

  /* build host array */
  int N  = outer[0];
  double *h_out = (double*) calloc(N, sizeof(double));

  /* allocate device array */
  occacMemory d_out = occacCreateBuffer(dev, N*sizeof(double), NULL);

  /* start kernel */
  double d = 12.34567;

  occacSetKernelArgBuf(simpleKernel, 0, d_out);
  occacSetKernelArgRaw(simpleKernel, 1, sizeof(d),     &d);
  occacSetKernelArgRaw(simpleKernel, 2, sizeof(N),     &N);

  occacQueueKernel(simpleKernel);

  /* copy results */
  occacToHost(d_out, h_out);

  /* output results */
  // for(int n=0;n<N;++n)
  //   printf("h_out[%d] = ", n, h_out[n]);
  printf("h_out[%d] = %g\n", N-1, h_out[N-1]);

  occacDestroyBuffer(dev, d_out);
  occacDestroyKernel(simpleKernel);
  occacDestroy(dev);
  free(h_out);
}
