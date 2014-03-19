#include "stdlib.h"
#include "occa.hpp"


int main(int argc, char **argv){

  int platform = 0, device = 0;

#if OCCA_USE_OPENCL==1
  void cl_list_all_devices();
  cl_list_all_devices();
#endif

  /* grab platform and device */
  occa dev;

  dev.setup("CPU", platform, device); // options CUDA, OpenCL, CPU

  /* build as cl kernel */
  occaKernel volumeKernel = dev.buildKernel("simple.occa", "simple", " ");

  /* set thread array for kernel */
  int dim = 1;
  size_t outer[3] = {100*128,1,1};
  size_t inner[3] = {128,1,1};
  volumeKernel.setThreadArray(outer, inner, dim);

  /* build host array */
  int N  = outer[0];
  double *h_out = (double*) calloc(N, sizeof(double));

  /* allocate device array */
  occaMemory d_out = dev.createBuffer(N*sizeof(double), NULL);

  /* start kernel */
  double d = 12.34567;
  volumeKernel(d_out, d, N);

  /* copy results */
  d_out.toHost(h_out);

  /* output results */
  //for(int n=0;n<N;++n)
  //  cout << "h_out[" << n << "] = " << h_out[n] << endl;

  cout << "h_out[" << N-1 << "] = " << h_out[N-1] << endl;

}
