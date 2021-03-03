SHELL = /bin/bash

C_COMPILE = /usr/bin/clang
CPP_COMPILE = /usr/bin/clang++
C_FLAGS = -O0 -Wall -g
CPP_FLAGS = -O0 -Wall -g -std=c++11
# C_COMPILE = /usr/bin/gcc
# CPP_COMPILE = /usr/bin/g++
# C_FLAGS = -Og -Wall -g
# CPP_FLAGS = -Og -Wall -g

INCLUDE = 
LIBS = -lm -lGL -lGLEW -lglfw

TARGETDIR = .
TARGET = ${TARGETDIR}/libmath_helpers.so
C_SOURCES = ./*.c
C_HEADERS = ./*.h
OBJECTS = *.o

all: ${TARGET}

clean_obj:
	rm -f *.o

${TARGET}: ${OBJECTS}
	${C_COMPILE} ${C_FLAGS} -shared -o $@ $^ ${LIBS}

${OBJECTS}: ${C_SOURCES} ${C_HEADERS}
	${C_COMPILE} ${C_FLAGS} -fPIC -c ${C_SOURCES} ${INCLUDE}

clean:
	rm -f *.o *~ ${TARGET}


