#include "occa.hpp"

occa::device dev;
occa::kernel add;
occa::memory d_a, d_b, d_c;

int a, b, c;

int main(int argc, char **argv){
  // setupAide setup("setuprc");

  occa::ompCompiler  = "clang++";

  occa::cudaCompiler = "nvcc";
  occa::cudaArch     = "35";
  occa::cudaCompilerFlags = "-ccbin /usr/bin/clang++";

  std::string mode;
  int device, platform;

  // setup.getArg("Mode"    , mode);
  // setup.getArg("Platform", platform);
  // setup.getArg("Device"  , device);

  dev.setup(mode, platform, device);

  std::cout << "dev.simdWidth() = " << dev.simdWidth() << '\n';

  occa::stream stream1 = dev.genStream();
  dev.setStream(stream1);

  add = dev.buildKernelFromSource("add.occa", "add");

  std::cout << "add.preferredDimSize() = " << add.preferredDimSize() << '\n';

  a = 1;
  b = 2;
  c = 0;

  d_a = dev.malloc(sizeof(int));
  d_b = dev.malloc(sizeof(int));
  d_c = dev.malloc(sizeof(int));

  d_a.copyFrom(&a);
  d_b.copyFrom(&b);

  add.setWorkingDims(1, occa::dim(1), occa::dim(1));

  add(d_a, d_b, d_c);

  d_c.copyTo(&c);

  std::cout << "c = " << c << '\n';

  return 0;
}
