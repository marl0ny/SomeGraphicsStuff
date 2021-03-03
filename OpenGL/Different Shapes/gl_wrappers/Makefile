SHELL = /bin/bash

C_COMPILE = /usr/bin/clang
CPP_COMPILE = /usr/bin/clang++
C_FLAGS = -O0 -Wall -g
CPP_FLAGS = -O0 -Wall -g -std=c++11
# C_COMPILE = /usr/bin/gcc
# CPP_COMPILE = /usr/bin/g++
# C_FLAGS = -Og -Wall -g
# CPP_FLAGS = -Og -Wall -g

INCLUDE = -I./gl_wrappers2d -I./math_helpers -I./shapes
LIBS = -lm -lGL -lGLEW -lglfw

TARGETDIR = .
TARGET = ${TARGETDIR}/libgl_wrappers.so
CPP_SOURCES = ./*.cpp
CPP_HEADERS = ./*.h
OBJECTS = *.o

all: ${TARGET}

clean_obj:
	rm -f *.o

${TARGET}: ${OBJECTS}
	${CPP_COMPILE} ${CPP_FLAGS} -shared -o $@ $^ ${LIBS}

${OBJECTS}: ${CPP_SOURCES} ${CPP_HEADERS}
	${CPP_COMPILE} ${CPP_FLAGS} -fPIC -c ${CPP_SOURCES} ${INCLUDE}

clean:
	rm -f *.o *.so ${TARGET}


