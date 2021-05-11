SHELL = /bin/bash

# COMPILE = /usr/bin/clang++
# FLAGS = -O2 -Wall -g -std=c++11 -pthread
COMPILE = /usr/bin/g++ 
FLAGS = -Og -Wall -g -pthread

INCLUDE = -fopenmp
LIBS = -lm -lGL -lGLEW -lglfw -fopenmp -fPIC

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

