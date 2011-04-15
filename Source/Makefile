build = Debug

ifeq ($(build), Debug)
	cflags = -g -Wall
	outdir = Debug
else ifeq ($(build), Release)
	cflags = -O3
	outdir = Release
endif

all: Make
	cd Make &&\
	g++ -c $(cflags) ../Engine/*/Source/*.cpp -I../Engine/Sound/Include -I../Engine/Font/Include &&\
	g++ -c $(cflags) ../Engine/OpenGL/Shaders/Preprocess.cpp &&\
	gcc -c $(cflags) ../Engine/Basic/LZMA/*.c &&\
	ar -rcs ../../$(outdir)/Linux/r5ge.a *.o

Make:
	mkdir Make

.PHONY: clean
.IGNORE: clean

clean:
	rm -r Make
