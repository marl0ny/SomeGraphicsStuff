
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
TARGET = ${TARGETDIR}/program
SOURCES = gl_wrappers.c waves.c
OBJECTS = gl_wrappers.o waves.o


all: ${TARGET}

${TARGET}: ${OBJECTS}
	clang ${FLAGS} -o $@ $^ ${LIBS}

${OBJECTS}: ${SOURCES}
	clang ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o ${TARGET}