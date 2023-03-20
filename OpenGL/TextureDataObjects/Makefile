SHELL = /bin/bash
FLAGS = -O0 -Wall -g
# FLAGS = -O2 -Wall
C_COMPILE = gcc
CPP_COMPILE = clang++ -std=c++17
# COMPILE = clang

# For Linux the prerequisite installs are OpenGL with GLEW and GLFW.
ifeq ($(shell uname), Linux)
INCLUDE =  -I${PWD} -I${PWD}/gl_wrappers
LIBS = -lm -lGL -lGLEW -lglfw
endif

# For MacOS you must first install the relavent OpenGL and GLFW libraries.
# If you installed the libraries through Homebrew you will probably first
# need to know the location of the Homebrew developement files, which varies
# from installation to installation, and then edit the INCLUDE and LIBS
# variables accordingly.
ifeq ($(shell uname),Darwin)
INCLUDE =  -I${PWD} -I${PWD}/gl_wrappers -I/opt/homebrew/include\
					 -I/usr/local/include
LIBS = -ldl -L/opt/homebrew/lib -L/usr/local/include -lglfw\
       -framework CoreVideo -framework OpenGL -framework IOKit\
       -framework Cocoa -framework Carbon
endif


TARGET = ${PWD}/program
C_SOURCES = ${PWD}/gl_wrappers/gl_wrappers.c frames_stacks.c bin_ops.c \
             unary_ops.c
C_OBJECTS = gl_wrappers.o frames_stacks.o bin_ops.o unary_ops.o
CPP_SOURCES = texture_data.cpp draw_texture_data.cpp main.cpp
CPP_OBJECTS = texture_data.o draw_texture_data.o main.o


all: ${TARGET}

${TARGET}: ${C_OBJECTS} ${CPP_OBJECTS}
	${CPP_COMPILE} ${FLAGS} -o $@ $^ ${LIBS}

${CPP_OBJECTS}: ${CPP_SOURCES}
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

${C_OBJECTS}: ${C_SOURCES}
	${C_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o ${TARGET}
