SHELL = /bin/bash
FLAGS = -O0 -Wall -g

# Put your own include and lib paths here:
INCLUDE = -I${HOME}/include -I${HOME}/Programs/include 
LIBS = -L${HOME}/lib -L${HOME}/Programs/lib -lm  -lGLESv2 -lGLEW -lglfw

TARGETDIR = .
# TARGETDIR = ${HOME}
TARGET = ${TARGETDIR}/world
SOURCES = world.c math_functions.c gl_wrappers.c vertex_array.c read_file.c
OBJECTS = world.o math_functions.o gl_wrappers.o vertex_array.o read_file.o
SHADERS = ./shaders/vertex_shader.vert ./shaders/fragment_shader.frag

all: ${TARGET}

${TARGET}: ${OBJECTS}
	cc ${FLAGS} -o $@ $^ ${LIBS}

${OBJECTS}: ${SOURCES}
	cc ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o *~ ${TARGET}

