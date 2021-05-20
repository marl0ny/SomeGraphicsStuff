SHELL = /bin/bash

# C_COMPILE = /usr/bin/clang
# CPP_COMPILE = /usr/bin/clang++
# C_FLAGS = -O0 -Wall -g
# CPP_FLAGS = -O0 -Wall -g -std=c++11
C_COMPILE = /usr/bin/gcc
CPP_COMPILE = /usr/bin/g++
C_FLAGS = -O2 -Wall -g
CPP_FLAGS = -O2 -Wall -g

INCLUDE = -fopenmp
LIBS = -lm -fPIC -fopenmp

TARGETDIR = .
TARGET = ${TARGETDIR}/libmarching_cubes.so
CPP_SOURCES = marching_cubes.cpp laplacian_smoothing.cpp 
CPP_HEADERS = marching_cubes.h laplacian_smoothing.h 
CPP_FILES = ${CPP_SOURCES} ${CPP_HEADERS} 
OBJECTS = *.o

all: ${TARGET}

clean_obj:
	rm -f *.o

${CPP_FILES}:
	git clone https://github.com/ilastik/marching_cubes
	cp ./marching_cubes/src/* ./

${TARGET}: ${OBJECTS}
	${CPP_COMPILE} ${CPP_FLAGS} -shared -o $@ $^ ${LIBS}

${OBJECTS}: ${CPP_FILES}
	${CPP_COMPILE} ${CPP_FLAGS} -fPIC -c ${CPP_SOURCES} ${INCLUDE}

clean:
	rm -f *.o *~ ${TARGET} 
	rm -f *.cpp *.h
	rm -rf ./*/


