SHELL = /bin/bash
FLAGS = -O0 -Wall -g

# Put your own include and lib paths here:
# INCLUDE = -I/opt/homebrew/include
# LIBS = -L/opt/homebrew/lib -lm  -lGLESv2 -lGLEW -lglfw\
#  -framework CoreVideo -framework OpenGL -framework IOKit\
#   -framework Cocoa -framework Carbon

INCLUDE = -I/opt/homebrew/include
LIBS = -L/opt/homebrew/lib -lglfw\
 -framework CoreVideo -framework OpenGL -framework IOKit\
  -framework Cocoa -framework Carbon


TARGETDIR = .
# TARGETDIR = ${HOME}
TARGET = ${TARGETDIR}/simple_cube
SOURCES = simple_cube.c simple_cube_helper_functions.c
OBJECTS = simple_cube.o simple_cube_helper_functions.o
SHADERS = vertex_shader.vert fragment_shader.frag

all: ${TARGET}

${TARGET}: ${OBJECTS}
	cc ${FLAGS} -o $@ $^ ${LIBS}

${OBJECTS}: ${SOURCES}
	cc ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o ${TARGET}

