build = Debug

ifeq ($(build), Debug)
	cflags = -g -Wall
	outdir = ../Debug/Linux
else ifeq ($(build), Release)
	cflags = -O3
	outdir = ../Release/Linux
endif

all: $(outdir)
	cd $(outdir) &&\
	g++ -c $(cflags) \
		-I../../Source/Engine/Sound/Include \
		-I../../Source/Engine/Font/Include \
		../../Source/Engine/*/Source/*.cpp &&\
	g++ -c $(cflags) \
		../../Source/Engine/OpenGL/Shaders/Preprocess.cpp &&\
	ar -rcs r5ge.a *.o

$(outdir):
	mkdir $(outdir)

.PHONY: clean
.IGNORE: clean

clean:
	rm 	../Debug/Linux/*.o \
		../Debug/Linux/r5ge.a \
		../Release/Linux/*.o \
		../Release/Linux/r5ge.a
