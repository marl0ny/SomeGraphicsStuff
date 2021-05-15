SHELL = /bin/bash

COMPILE = /usr/bin/clang++
FLAGS = -O0 -Wall -g -std=c++11
# COMPILE = /usr/bin/g++ 
# FLAGS = -Og -Wall -g

INCLUDE = 
LIBS = -lm -lGL -lGLEW -lglfw

TARGETDIR = .
TARGET = ${TARGETDIR}/wave
SOURCES = wave.cpp gl_wrappers.cpp
OBJECTS = wave.o gl_wrappers.o

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${COMPILE} ${FLAGS} -o $@ $^ ${LIBS}

${OBJECTS}: ${SOURCES}
	${COMPILE} ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o *~ ${TARGET}

