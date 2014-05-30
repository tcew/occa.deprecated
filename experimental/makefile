include ${OCCA_DIR}/scripts/makefile

#---[ WORKING PATHS ]-----------------------------
lPath = lib
#=================================================

#---[ COMPILATION ]-------------------------------
headers = $(wildcard $(iPath)/*.hpp) $(wildcard $(iPath)/*.tpp)
sources = $(wildcard $(sPath)/*.cpp)

objects = $(subst $(sPath)/,$(oPath)/,$(sources:.cpp=.o))

$(lPath)/libocca.a:$(objects) $(headers)
	ar rcs $(lPath)/libocca.a $(objects)

$(oPath)/%.o:$(sPath)/%.cpp $(iPath)/%.hpp$(wildcard $(subst $(sPath)/,$(iPath)/,$(<:.cpp=.hpp))) $(wildcard $(subst $(sPath)/,$(iPath)/,$(<:.cpp=.tpp)))\
	$(oPath)/occaKernelDefines.o
	$(compiler) $(compilerFlags) -o $@ $(flags) -c $(paths) $<

$(oPath)/occaKernelDefines.o:$(iPath)/occaOpenMPDefines.hpp $(iPath)/occaOpenCLDefines.hpp $(iPath)/occaCUDADefines.hpp $(iPath)/occaKernelDefines.hpp
	$(compiler) $(compilerFlags) -o $(oPath)/occaKernelDefines.o $(flags) -c $(paths) $(sPath)/occaKernelDefines.cpp

$(iPath)/occaKernelDefines.hpp:
	$(compiler) -o $(OCCA_DIR)/scripts/main $(OCCA_DIR)/scripts/main.cpp
	$(OCCA_DIR)/scripts/main

clean:
	rm -f $(oPath)/*;
	rm -f lib/libocca.a;
#=================================================
