SHELL = /bin/bash
FLAGS = -O0 -g -Wno-everything # -Wall
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
             unary_ops.c fft.c bitmap.c summation.c
C_OBJECTS = gl_wrappers.o frames_stacks.o bin_ops.o unary_ops.o fft.o \
			 bitmap.o summation.o
CPP_SOURCES_DEP = texture_data.cpp draw_texture_data.cpp
CPP_OBJECTS_DEP = texture_data.o draw_texture_data.o
CPP_SOURCES_SIM = texture_data_transfers_stuff.cpp \
					summation_stuff.cpp reduction_to_1d.cpp \
					complex_func.cpp \
					waves.cpp poisson.cpp \
					reaction_diffusion.cpp stable_fluids.cpp \
					n_body.cpp mol_dynamics.cpp \
					schrod_leapfrog.cpp schrod_splitstep.cpp \
					schrod_splitstep_image_potential.cpp \
					dirac_leapfrog.cpp isf_splitstep.cpp
CPP_OBJECTS_SIM = texture_data_transfers_stuff.o summation_stuff.o \
					reduction_to_1d.o complex_func.o waves.o\
					poisson.o reaction_diffusion.o stable_fluids.o \
					n_body.o mol_dynamics.o \
					schrod_splitstep_image_potential.o schrod_leapfrog.o\
					schrod_splitstep.cpp isf_splitstep.o\
					dirac_leapfrog.o
CPP_SOURCES_MAIN = main.cpp
CPP_OBJECTS_MAIN =  main.o


all: ${TARGET}

${TARGET}: ${C_OBJECTS} ${CPP_OBJECTS_DEP} ${CPP_OBJECTS_SIM} ${CPP_OBJECTS_MAIN}
	${CPP_COMPILE} ${FLAGS} -o $@ $^ ${LIBS} ${INCLUDE}

${CPP_OBJECTS_MAIN}: ${CPP_SOURCES_MAIN}
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

${CPP_OBJECTS_SIM}: ${CPP_SOURCES_SIM}
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

${CPP_OBJECTS_DEP}: ${CPP_SOURCES_DEP}
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

${C_OBJECTS}: ${C_SOURCES}
	${C_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o ${TARGET}
