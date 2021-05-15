SHELL = /bin/bash

COMPILE = /usr/bin/clang
FLAGS = -O0 -Wall -g
# COMPILE = /usr/bin/g++ 
# FLAGS = -Og -Wall -g


# Put your own include and lib paths here:
INCLUDE = 
LIBS = -lm  -lGLESv2 -lGLEW -lglfw

TARGETDIR = .
# TARGETDIR = ${HOME}
TARGET = ${TARGETDIR}/simple_triangle
SOURCES = simple_triangle.c
OBJECTS = simple_triangle.o

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${COMPILE} ${FLAGS} -o $@ $^ ${LIBS}

${OBJECTS}: ${SOURCES}
	${COMPILE} ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o ${TARGET}

