SHELL = /bin/bash

# C_COMPILE = /usr/bin/clang
# CPP_COMPILE = /usr/bin/clang++
# C_FLAGS = -O2 -Wall -g
# CPP_FLAGS = -O2 -Wall -g -std=c++11
C_COMPILE = /usr/bin/gcc
CPP_COMPILE = /usr/bin/g++
C_FLAGS = -O2 -Wall -g
CPP_FLAGS = -O2 -Wall -g
OPENMP = -fopenmp

INCLUDE = -I./gl_wrappers ${OPENMP}
LIBS = -lm -lGL -lGLEW -lglfw -fPIC ${OPENMP}

TARGETDIR = .
TARGET = ${TARGETDIR}/program
CPP_SOURCES = *.cpp
CPP_HEADERS = *.h
SOURCES = ${CPP_SOURCES}
OBJECTS = *.o

SUBDIRS = ./gl_wrappers ./math_helpers ./marching_cubes ./spherical_harmonics
SO_FILES = ./gl_wrappers/libgl_wrappers.so ./math_helpers/libmath_helpers.so \
					 ./marching_cubes/libmarching_cubes.so \
					 ./spherical_harmonics/spherical_harmonics.so

all: ${TARGET}

.PHONY: ${SUBDIRS} clean clean_all

clean_obj:
	rm -f *.o

${TARGET}: ${OBJECTS} ${SO_FILES}
	${CPP_COMPILE} ${CPP_FLAGS} -o $@ $^ ${LIBS} ${SO_FILES}

${SUBDIRS}:
	${MAKE} -C $@

${SO_FILES}: ${SUBDIRS}

${OBJECTS}: ${SOURCES} ${CPP_HEADERS} ${SO_FILES}
	${CPP_COMPILE} ${CPP_FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o *~ *.gch ${TARGET}

clean_all:
	rm -f *.o ./*/*.o ./*/*.so *~ *.gch ${TARGET}

clean_all_objs:
	rm -f *.o ./*/*.o *~ *.gch

