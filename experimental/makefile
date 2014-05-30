include ./scripts/makefile

#---[ WORKING PATHS ]-----------------------------
lPath = lib
#=================================================

#---[ COMPILATION ]-------------------------------
headers = $(wildcard $(iPath)/*.hpp) $(wildcard $(iPath)/*.tpp)
sources = $(wildcard $(sPath)/*.cpp)

objects  = $(subst $(sPath)/,$(oPath)/,$(sources:.cpp=.o))

$(lPath)/libocca.a: $(objects) $(headers)
	ar rcs $(lPath)/libocca.a $(objects)

$(oPath)/%.o:$(sPath)/%.cpp $(iPath)/%.hpp$(wildcard $(subst $(sPath)/,$(iPath)/,$(<:.cpp=.hpp))) $(wildcard $(subst $(sPath)/,$(iPath)/,$(<:.cpp=.tpp)))
	$(compiler) $(compilerFlags) -o $@ $(flags) -c $(paths) $<

clean:
	rm -f $(oPath)/*;
	rm -f lib/libocca.a;
#=================================================
