SHELL = /bin/bash

# COMPILE = /usr/bin/clang++
# FLAGS = -O2 -Wall -g -std=c++11
COMPILE = /usr/bin/g++ 
FLAGS = -Og -Wall -g

OPENMP = -fopenmp
INCLUDE = ${OPENMP}
LIBS = -lm -lGL -lGLEW -lglfw -fPIC ${OPENMP}

TARGETDIR = .
TARGET = ${TARGETDIR}/program
SOURCES = program.cpp gl_wrappers.cpp fft.hpp
OBJECTS = program.o gl_wrappers.o # fft.o

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${COMPILE} ${FLAGS} -o $@ $^ ${LIBS}

${OBJECTS}: ${SOURCES}
	${COMPILE} ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o *~ *.gch ${TARGET}

