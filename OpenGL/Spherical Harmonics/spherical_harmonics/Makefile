SHELL = /bin/bash

C_COMPILE = /usr/bin/clang
CPP_COMPILE = /usr/bin/clang++
C_FLAGS = -O2 -Wall -g
CPP_FLAGS = -O2 -Wall -g -std=c++11
# C_COMPILE = /usr/bin/gcc
# CPP_COMPILE = /usr/bin/g++
# C_FLAGS = -Og -Wall -g
# CPP_FLAGS = -Og -Wall -g

INCLUDE = 
LIBS = -lm -lGL -lGLEW -lglfw

TARGETDIR = .
TARGET = ${TARGETDIR}/spherical_harmonics.so
CPP_SOURCES = ./harmonics.cpp
CPP_HEADERS = ./harmonics.h
OBJECTS = *.o

all: ${TARGET}

clean_obj:
	rm -f *.o

${CPP_SOURCES}:
	python3 write_harmonics.py

${TARGET}: ${OBJECTS}
	${CPP_COMPILE} ${CPP_FLAGS} -shared -o $@ $^ ${LIBS}

${OBJECTS}: ${CPP_SOURCES} ${CPP_HEADERS}
	${CPP_COMPILE} ${CPP_FLAGS} -fPIC -c ${CPP_SOURCES} ${INCLUDE}

clean:
	rm -f *.o *~ ${TARGET}
	rm  ${CPP_SOURCES}


