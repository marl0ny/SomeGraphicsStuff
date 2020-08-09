SHELL = /bin/bash
FLAGS = -O2 -Wall -g

# Put your own include and lib paths here:
INCLUDE = -I${HOME}/include -I${HOME}/Programs/include 
LIBS = -L${HOME}/lib -L${HOME}/Programs/lib -lm  -lGLESv2 -lGLEW -lglfw

TARGETDIR = .
# TARGETDIR = ${HOME}
TARGET = ${TARGETDIR}/simple_triangle
SOURCES = simple_triangle.c
OBJECTS = simple_triangle.o

all: ${TARGET}

${TARGET}: ${OBJECTS}
	cc ${FLAGS} -o $@ $^ ${LIBS}

${OBJECTS}: ${SOURCES}
	cc ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o ${TARGET}

