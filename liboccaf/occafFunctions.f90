module occafFunctions_m
  use occafTypes_m

  implicit none

  private

  public ::                             &
    occafBuildKernel,                   &
    occafCreateBuffer,                  &
    occafDestroy,                       &
    occafDestroyBuffer,                 &
    occafDestroyKernel,                 &
    occafNew,                           &
    occafQueueKernel,                   &
    occafSetKernelArg,                  &
    occafSetThreadArray,                &
    occafSetup,                         &
    occafToHost

  interface occafBuildKernel
    module procedure occafBuildKernel_func
  end interface occafBuildKernel

  interface occafCreateBuffer
    module procedure occafCreateBuffer_null
    module procedure occafCreateBuffer_int4
    module procedure occafCreateBuffer_int8
    module procedure occafCreateBuffer_real4
    module procedure occafCreateBuffer_real8
    module procedure occafCreateBuffer_char
  end interface occafCreateBuffer

  interface occafDestroyBuffer
    subroutine occafDestroyBuffer_c(c, buf)
      use occafTypes_m
      implicit none
      type(occafState),  intent(in)    :: c
      type(occafMemory), intent(inout) :: buf
    end subroutine occafDestroyBuffer_c
  end interface occafDestroyBuffer

  interface occafDestroyKernel
    subroutine occafDestroyKernel_c(kernel)
      use occafTypes_m
      implicit none
      type(occafKernel),  intent(inout) :: kernel
    end subroutine occafDestroyKernel_c
  end interface occafDestroyKernel

  interface occafDestroy
    subroutine occafDestroy_c(c)
      use occafTypes_m
      implicit none
      type(occafState),  intent(inout) :: c
    end subroutine occafDestroy_c
  end interface occafDestroy

  interface occafNew
    module procedure occafNew_func
  end interface occafNew

  interface occafQueueKernel
    subroutine occafQueueKernel_c(kernel)
      use occafTypes_m
      implicit none
      type(occafKernel), intent(in) :: kernel
    end subroutine occafQueueKernel_c
  end interface occafQueueKernel

  interface occafSetKernelArg
    subroutine occafSetKernelArg_mem_c(kernel, idx, arg)
      use occafTypes_m
      implicit none
      type(occafKernel), intent(inout) :: kernel
      integer,           intent(in)    :: idx
      type(occafMemory), intent(in)    :: arg
    end subroutine occafSetKernelArg_mem_c

    subroutine occafSetKernelArg_int4_c(kernel, idx, arg)
      use occafTypes_m
      implicit none
      type(occafKernel), intent(inout) :: kernel
      integer,           intent(in)    :: idx
      integer(4),        intent(in)    :: arg
    end subroutine occafSetKernelArg_int4_c

    subroutine occafSetKernelArg_int8_c(kernel, idx, arg)
      use occafTypes_m
      implicit none
      type(occafKernel), intent(inout) :: kernel
      integer,           intent(in)    :: idx
      integer(8),        intent(in)    :: arg
    end subroutine occafSetKernelArg_int8_c

    subroutine occafSetKernelArg_real4_c(kernel, idx, arg)
      use occafTypes_m
      implicit none
      type(occafKernel), intent(inout) :: kernel
      integer,           intent(in)    :: idx
      real(4),           intent(in)    :: arg
    end subroutine occafSetKernelArg_real4_c

    subroutine occafSetKernelArg_real8_c(kernel, idx, arg)
      use occafTypes_m
      implicit none
      type(occafKernel), intent(inout) :: kernel
      integer,           intent(in)    :: idx
      real(8),           intent(in)    :: arg
    end subroutine occafSetKernelArg_real8_c

    subroutine occafSetKernelArg_char_c(kernel, idx, arg)
      use occafTypes_m
      implicit none
      type(occafKernel), intent(inout) :: kernel
      integer,           intent(in)    :: idx
      character,         intent(in)    :: arg
    end subroutine occafSetKernelArg_char_c
  end interface occafSetKernelArg

  interface occafSetThreadArray
    subroutine occafSetThreadArray_c(kernel, global, local, tdim)
      use occafTypes_m
      implicit none
      type(occafKernel), intent(inout) :: kernel
      integer(8),        intent(in)    :: global(3)
      integer(8),        intent(in)    :: local(3)
      integer,           intent(in)    :: tdim
    end subroutine occafSetThreadArray_c
  end interface occafSetThreadArray

  interface occafSetup
    subroutine occafSetup_c(c, threadModel, platform, device)
      use occafTypes_m
      implicit none
      type(occafState), intent(inout) :: c
      character(len=*), intent(in)    :: threadModel
      integer,          intent(in)    :: platform
      integer,          intent(in)    :: device
    end subroutine occafSetup_c
  end interface occafSetup

  interface occafToHost
    subroutine occafToHost_int4_c(mem, arg)
      use occafTypes_m
      implicit none
      type(occafMemory), intent(in) :: mem
      integer(4),        intent(in) :: arg
    end subroutine occafToHost_int4_c

    subroutine occafToHost_int8_c(mem, arg)
      use occafTypes_m
      implicit none
      type(occafMemory), intent(in) :: mem
      integer(8),        intent(in) :: arg
    end subroutine occafToHost_int8_c

    subroutine occafToHost_real4_c(mem, arg)
      use occafTypes_m
      implicit none
      type(occafMemory), intent(in) :: mem
      real(4),           intent(in) :: arg
    end subroutine occafToHost_real4_c

    subroutine occafToHost_real8_c(mem, arg)
      use occafTypes_m
      implicit none
      type(occafMemory), intent(in) :: mem
      real(8),           intent(in) :: arg
    end subroutine occafToHost_real8_c

    subroutine occafToHost_char_c(mem, arg)
      use occafTypes_m
      implicit none
      type(occafMemory), intent(in) :: mem
      character,         intent(in) :: arg
    end subroutine occafToHost_char_c
  end interface occafToHost

contains

  type(occafKernel) function occafBuildKernel_func(c, filename, kernelname, deffile, flags) result(kernel)
    type(occafState),  intent(inout) :: c
    character(len=*),  intent(in)    :: filename
    character(len=*),  intent(in)    :: kernelname
    character(len=*),  intent(in)    :: deffile
    character(len=*),  intent(in)    :: flags

    interface
      subroutine occafBuildKernel_c(kernel, c, filename, kernelname, deffile, flags)
        use occafTypes_m
        implicit none
        type(occafKernel), intent(out)   :: kernel
        type(occafState),  intent(inout) :: c
        character(len=*),  intent(in)    :: filename
        character(len=*),  intent(in)    :: kernelname
        character(len=*),  intent(in)    :: deffile
        character(len=*),  intent(in)    :: flags
      end subroutine occafBuildKernel_c
    end interface

    call occafBuildKernel_c(kernel, c, filename, kernelname, deffile, flags)
  end function occafBuildKernel_func

  type(occafMemory) function occafCreateBuffer_null(c, sz) result(mem)
    type(occafState),  intent(inout) :: c
    integer(8),        intent(in)    :: sz

    call occafCreateBuffer_NULL_c(mem, c, sz)
  end function occafCreateBuffer_null

  type(occafMemory) function occafCreateBuffer_int4(c, sz, buf) result(mem)
    type(occafState),  intent(inout) :: c
    integer(8),        intent(in)    :: sz
    integer(4),        intent(in)    :: buf

    call occafCreateBuffer_c(mem, c, sz, buf)
  end function occafCreateBuffer_int4
  type(occafMemory) function occafCreateBuffer_int8(c, sz, buf) result(mem)
    type(occafState),  intent(inout) :: c
    integer(8),        intent(in)    :: sz
    integer(8),        intent(in)    :: buf

    call occafCreateBuffer_c(mem, c, sz, buf)
  end function occafCreateBuffer_int8
  type(occafMemory) function occafCreateBuffer_real4(c, sz, buf) result(mem)
    type(occafState),  intent(inout) :: c
    integer(8),        intent(in)    :: sz
    real(4),           intent(in)    :: buf

    call occafCreateBuffer_c(mem, c, sz, buf)
  end function occafCreateBuffer_real4
  type(occafMemory) function occafCreateBuffer_real8(c, sz, buf) result(mem)
    type(occafState),  intent(inout) :: c
    integer(8),        intent(in)    :: sz
    real(8),           intent(in)    :: buf

    call occafCreateBuffer_c(mem, c, sz, buf)
  end function occafCreateBuffer_real8
  type(occafMemory) function occafCreateBuffer_char(c, sz, buf) result(mem)
    type(occafState),  intent(inout) :: c
    integer(8),        intent(in)    :: sz
    character,         intent(in)    :: buf

    call occafCreateBuffer_c(mem, c, sz, buf)
  end function occafCreateBuffer_char

  type(occafState) function occafNew_func() result(state)
    interface
      subroutine occafNew_c(state)
        use occafTypes_m
        implicit none
        type(occafState), intent(out) :: state
      end subroutine occafNew_c

    end interface

    call occafNew_c(state)
  end function occafNew_func

end module occafFunctions_m
