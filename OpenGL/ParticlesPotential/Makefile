MACOSX_HOMEBREW = yes

SHELL = /bin/bash
FLAGS = -O0 -Wall -g
COMPILE = gcc

ifdef MACOSX_HOMEBREW
INCLUDE =  -I${PWD} -I${PWD}/gl_wrappers -I/opt/homebrew/include
LIBS = -ldl -L/opt/homebrew/lib -lglfw\
       -framework CoreVideo -framework OpenGL -framework IOKit\
       -framework Cocoa -framework Carbon
else
INCLUDE =  -I${PWD} -I${PWD}/gl_wrappers
LIBS = -lm -lGL -lGLEW -lglfw
endif


TARGET = ${PWD}/program
SOURCES = ${PWD}/gl_wrappers/gl_wrappers.c cg_gl.c summation_gl.c\
	fft_gl.c init_render.c main.c
OBJECTS = gl_wrappers.o cg_gl.o summation_gl.o init_render.o fft_gl.o main.o


all: ${TARGET}

${TARGET}: ${OBJECTS}
	${COMPILE} ${FLAGS} -o $@ $^ ${LIBS}

${OBJECTS}: ${SOURCES}
	${COMPILE} ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o ${TARGET}
