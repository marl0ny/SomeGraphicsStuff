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
TARGET = ${TARGETDIR}/world
SOURCES = world.c math_functions.c gl_wrappers.c vertex_array.c read_file.c
OBJECTS = world.o math_functions.o gl_wrappers.o vertex_array.o read_file.o
SHADERS = ./shaders/vertex_shader.vert ./shaders/fragment_shader.frag

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${COMPILE} ${FLAGS} -o $@ $^ ${LIBS}

${OBJECTS}: ${SOURCES}
	${COMPILE} ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o *~ ${TARGET}

