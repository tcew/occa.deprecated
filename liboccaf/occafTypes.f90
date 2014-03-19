module occafTypes_m
  implicit none

  type :: occafState
    private
    integer, pointer :: p
  end type occafState

  type :: occafKernel
    private
    integer, pointer :: p
  end type occafKernel

  type :: occafMemory
    private
    integer, pointer :: p
  end type occafMemory

end module occafTypes_m
