# Default OCCA variables
enableOpenCL = 1
enableCUDA   = 1
enableGL     = 0
enableMPI    = 0

compiler = g++

# OCCA Object/Source/Include directories
occaIPath = libocca
occaSPath = libocca
occaOPath = libocca

# Paths
paths += -I./$(occaSPath)
paths += -I./include -I/opt/AMDAPP/include -I/usr/local/cuda/include -m64

links = -L. -L./lib -L/usr/local/cuda/lib

# OS Detection
ifdef SystemRoot  #---[ WINDOWS ]-------

flags += -D OS_WINDOWS=1

else #---[ OSX ]------------------------
ifeq ($(shell uname -s),Darwin)

flags += -D OS_OSX=1

else #---[ LINUX ]----------------------

flags += -D OS_LINUX=1

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

ifeq ($(enableMPI), 1)
	flags += -DOCCA_MPI_ENABLED=1
else
	flags += -DOCCA_MPI_ENABLED=0
endif

flags += -DOCCA_USE_CPU=1

occaHeaders = $(wildcard $(occaIPath)/*.hpp) $(wildcard $(occaIPath)/*.tpp)
occaSources = $(wildcard $(occaSPath)/*.cpp)
occaObjects = $(subst $(occaSPath),$(occaOPath),$(occaSources:.cpp=.o))

$(occaOPath)/libocca.a: $(occaObjects) $(occaHeaders)
	ar rcs $(occaOPath)/libocca.a $(occaObjects)

$(occaOPath)/%.o:$(occaSPath)/%.cpp $(wildcard $(subst $(occaSPath),$(occaIPath),$(<:.cpp=.hpp))) $(wildcard $(subst $(occaSPath),$(occaIPath),$(<:.cpp=.tpp)))
	$(compiler) -o $@ $(flags) $(paths) -c $<

clean:
	rm -f $(occaOPath)/*.o;
	rm -f $(occaOPath)/*.a;
