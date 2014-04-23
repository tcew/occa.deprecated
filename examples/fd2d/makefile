# Default variables
enableOpenCL = 1
enableCUDA   = 1
enableGL     = 0

# Libocca path
occaPath = ./libocca

# Paths
paths  = -I./$(sPath) -I./$(iPath)
paths += -I$(occaPath)
paths += -I./include -I/opt/AMDAPP/include -I/usr/local/cuda/include -m64

links = -L. -L/usr/local/cuda/lib -L$(occaPath) -locca

flags = -Ddatafloat=float

#---[ OPTIONS ]-----------------------------------
#
#  MPI=0     : Disables MPI
#  DEBUG=0   : Uses optimization flags
#  SINGLE=1  : Single precision, else double precision
#  DOUBLE=1  : Double precision, else single precision
#  GL=0      : Disables OpenGL based code
#
#---[ MESSY STUFF BELOW ]-------------------------

# Object/Include directories
oPath = obj
sPath = src
iPath = include

occaIPath = libocca
occaSPath = libocca
occaOPath = libocca

# Ignore the nasty OpenGL deprecated messages from OS 10.9
flags += -Wno-deprecated

# ---[ Cluster ]------------------------
HOSTNAME = $(shell hostname)
ifeq ($(findstring davinci,$(HOSTNAME)),davinci)
	GL = 0
endif
# ======================================

ifeq ($(MPI), 0)
	flags += -DMPI_ENABLED=0
	compiler = $(CXX)
else
	flags += -DMPI_ENABLED=1
	compiler = $(MPICXX)
endif

ifeq ($(DEBUG), 1)
	flags += -g
else
	flags += -DNDEBUG=1 -mtune=native -O3 -ftree-vectorize -funroll-all-loops -ffast-math # -fsplit-ivs-in-unroller
endif

# # Float/Double
# ifeq ($(DOUBLE), 1)
# 	flags += -Ddatafloat=double
# else
# 	flags += -Ddatafloat=float
# endif

# OS Detection
ifdef SystemRoot  #---[ WINDOWS ]-------

flags += -D OS_WINDOWS=1

else #---[ OSX ]------------------------

ifeq ($(shell uname -s),Darwin)

ifneq ($(GL), 0)
	links += -framework GLUT -framework OpenGL
endif

ifeq ($(enableOpenCL), 1)
	links += -framework OpenCL
endif

flags += -D OS_OSX=1
links += -framework vecLib  -framework CoreServices

ifeq ($(enableCUDA), 1)
	links += -lcuda
endif

else #---[ LINUX ]----------------------

flags += -D OS_LINUX=1

HOSTNAME = $(shell hostname)
ifeq ($(findstring davinci,$(HOSTNAME)),davinci)
	paths += -L/opt/apps/intel/2011.0.013/mkl/lib/intel64
	links += -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -openmp -lpthread
	links += -lm -lgfortran -lrt -ldl
else
	links += -llapack -lblas -lm -lgfortran -lrt -ldl
endif

ifneq ($(GL), 0)
	links += -lglut -lGL -lGLU -lGLEW
endif

ifeq ($(enableOpenCL), 1)
	links += -lOpenCL
endif

ifeq ($(enableCUDA), 1)
	links += -lcuda
endif

endif
endif

#---[ OCCA ]--------------------------------------

ifeq ($(enableOpenCL), 1)
	flags += -DOCCA_USE_OPENCL=1

	ifeq ($(enableCUDA), 1)
		flags += -DOCCA_USE_CUDA=1
		flags += -DOCCA_USE_ALL=1
	else
		flags += -DOCCA_USE_CUDA=0
		flags += -DOCCA_USE_ALL=0
	endif
else
	flags += -DOCCA_USE_OPENCL=0
	flags += -DOCCA_USE_ALL=0

	ifeq ($(enableCUDA), 1)
	flags += -DOCCA_USE_CUDA=1
	else
	flags += -DOCCA_USE_CUDA=0
	endif
endif

ifeq ($(enableGL), 1)
	flags += -DCOMPILED_WITH_GL=1
else
	flags += -DCOMPILED_WITH_GL=0
endif

flags += -DOCCA_USE_CPU=1

# Substitutes files with the includes/objects paths
headers = $(wildcard $(iPath)/*.hpp) $(wildcard $(iPath)/*.tpp)
sources = $(wildcard $(sPath)/*.cpp)
objects = $(subst $(sPath),$(oPath),$(sources:.cpp=.o))

occaHeaders = $(wildcard $(occaIPath)/*.hpp) $(wildcard $(occaIPath)/*.tpp)
occaSources = $(wildcard $(occaSPath)/*.cpp)
occaObjects = $(subst $(occaSPath),$(occaOPath),$(occaSources:.cpp=.o))

main: $(objects) $(occaObjects) $(headers) $(occaHeaders) main.cpp
	$(compiler) -o main $(flags) $(objects) $(occaObjects) main.cpp $(paths) $(links)

$(oPath)/%.o:$(sPath)/%.cpp $(wildcard $(subst $(sPath),$(iPath),$(<:.cpp=.hpp))) $(wildcard $(subst $(sPath),$(iPath),$(<:.cpp=.tpp)))
	$(compiler) -o $@ $(flags) $(paths) -c $<

$(occaOPath)/%.o:$(occaSPath)/%.cpp $(wildcard $(subst $(occaSPath),$(occaIPath),$(<:.cpp=.hpp))) $(wildcard $(subst $(occaSPath),$(occaIPath),$(<:.cpp=.tpp)))
	$(compiler) -o $@ $(flags) $(paths) -c $<

clean:
	rm -f $(oPath)/*.o;
	rm -f $(occaOPath)/*.o;
	rm -f main;
	rm -f .occa/*rank/*;
