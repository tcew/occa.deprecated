!! Simple Fortran OCCA example

program simple
  use occaf
  implicit none

  type(occafState) :: dev

  integer :: platform = 0, device = 0

  integer :: tdim
  integer(8) :: outer(3), inner(3)
  integer(8) :: N, sz_bytes

  integer :: AllocateStatus, DeAllocateStatus

  real(8), allocatable :: h_out(:)
  real(8) :: d

  type(occafKernel) :: simpleKernel
  type(occafMemory) :: d_out

  dev = occafNew()

  call occafSetup(dev, 'CUDA', platform, device)

  simpleKernel = occafBuildKernel(dev, 'simple.occa', 'simple', '', '');

  tdim = 1
  outer = (/ 100*128, 1, 1 /)
  inner = (/     128, 1, 1 /)
  call occafSetThreadArray(simpleKernel, outer, inner, tdim)

  N = outer(1)
  allocate(h_out(1:N), STAT = AllocateStatus)
  if(AllocateStatus /= 0) stop '*** Allocation Fail ***'

  sz_bytes = int(N, 8) * 8_8
  d_out = occafCreateBuffer(dev, sz_bytes)

  d = 12.34567d0

  call occafSetKernelArg(simpleKernel, 0, d_out)
  call occafSetKernelArg(simpleKernel, 1, d)
  call occafSetKernelArg(simpleKernel, 2, N)

  call occafQueueKernel(simpleKernel)

  call occafToHost(d_out, h_out(1))

  print *,"h_out(", N, ") = ", h_out(N)

  call occafDestroyBuffer(dev, d_out);
  call occafDestroyKernel(simpleKernel);
  call occafDestroy(dev);

  deallocate(h_out, STAT = DeAllocateStatus)
  if(DeAllocateStatus /= 0) stop '*** Deallocation Fail ***'
end program simple
