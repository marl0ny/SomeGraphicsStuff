
SHELL = /bin/bash
FLAGS = -O0 -Wall -g

# Put your own include and lib paths here:

# INCLUDE = 
# LIBS = -lm  -lGLESv2 -lGLEW -lglfw
# COMPILE = clang

INCLUDE = -I/opt/homebrew/include
LIBS = -L/opt/homebrew/lib -lglfw\
 -framework CoreVideo -framework OpenGL -framework IOKit\
  -framework Cocoa -framework Carbon
COMPILE = clang

TARGETDIR = .
TARGET = ${TARGETDIR}/program
SOURCES = gl_wrappers.c waves.c
OBJECTS = gl_wrappers.o waves.o


all: ${TARGET}

${TARGET}: ${OBJECTS}
	${COMPILE} ${FLAGS} -o $@ $^ ${LIBS}

${OBJECTS}: ${SOURCES}
	${COMPILE} ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o ${TARGET}