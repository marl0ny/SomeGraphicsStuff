SHELL = /bin/bash

COMPILE = /usr/bin/clang++
FLAGS = -O2 -Wall -g -std=c++17
# COMPILE = /usr/bin/g++ 
# FLAGS = -Og -Wall -g

INCLUDE = -I/opt/homebrew/include
LIBS = -L/opt/homebrew/lib -lglfw\
 -framework CoreVideo -framework OpenGL -framework IOKit\
  -framework Cocoa -framework Carbon

TARGETDIR = .
TARGET = ${TARGETDIR}/wave
SOURCES = wave.cpp gl_wrappers.cpp
OBJECTS = wave.o gl_wrappers.o

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${COMPILE} ${FLAGS} -o $@ $^ ${LIBS}

${OBJECTS}: ${SOURCES}
	${COMPILE} ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o *~ ${TARGET}

