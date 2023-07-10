SHELL = /bin/bash
FLAGS = -O2 -g -Wno-everything -ffast-math # -Wall
# FLAGS = -O2 -Wall
C_COMPILE = clang 
CPP_COMPILE = clang++ -std=c++11
# COMPILE = clang

# For Linux the prerequisite installs are OpenGL with GLEW and GLFW.
ifeq ($(shell uname), Linux)
INCLUDE =  -I${PWD} -I${PWD}/gl_wrappers
LIBS = -lm -lGL -lGLEW -lglfw -pthread -lpng
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
       -framework Cocoa -framework Carbon -framework Accelerate\
	   -lpng
endif


TARGET = ${PWD}/program
C_SOURCES = ${PWD}/gl_wrappers/gl_wrappers.c frames_stacks.c bin_ops.c \
             unary_ops.c fft.c bitmap.c summation.c write_to_png.c \
						 read_from_png.c serialize.c
C_OBJECTS = gl_wrappers.o frames_stacks.o bin_ops.o unary_ops.o fft.o \
			 bitmap.o summation.o write_to_png.o read_from_png.o serialize.o
CPP_SOURCES = complex_func.cpp dirac_leapfrog.cpp draw_texture_data.cpp\
							isf_splitstep.cpp main.cpp mol_dynamics.cpp mol_dynamics_mt.cpp \
							n_body.cpp poisson.cpp reaction_diffusion.cpp \
							reduction_to_1d.cpp schrod_leapfrog.cpp schrod_splitstep.cpp \
							schrod_splitstep_image_potential.cpp stable_fluids.cpp \
							summation_stuff.cpp texture_data.cpp fft_omp.cpp \
							texture_data_transfers_stuff.cpp waves.cpp sph.cpp
CPP_OBJECTS = texture_data.o draw_texture_data.o \
					texture_data_transfers_stuff.o summation_stuff.o \
					reduction_to_1d.o complex_func.o waves.o \
					poisson.o reaction_diffusion.o stable_fluids.o \
					n_body.o mol_dynamics.o mol_dynamics_mt.o sph.o \
					schrod_splitstep_image_potential.o schrod_leapfrog.o\
					schrod_splitstep.o isf_splitstep.o \
					dirac_leapfrog.o fft_omp.o  \
				  main.o
OBJECTS = ${C_OBJECTS} ${CPP_OBJECTS}

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${CPP_COMPILE} ${FLAGS} -o $@ $^ ${LIBS} ${INCLUDE}

# for e in $(find *.cpp); \
do printf $e | sed 's/\.cpp/\.o:/g'; echo ' '$e; \
printf '\t${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}' ; echo; echo; done


complex_func.o: complex_func.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

dirac_leapfrog.o: dirac_leapfrog.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

draw_texture_data.o: draw_texture_data.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

fft_omp.o: fft_omp.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

isf_splitstep.o: isf_splitstep.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

main.o: main.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

mol_dynamics.o: mol_dynamics.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

mol_dynamics_mt.o: mol_dynamics_mt.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

n_body.o: n_body.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

poisson.o: poisson.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

reaction_diffusion.o: reaction_diffusion.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

reduction_to_1d.o: reduction_to_1d.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

schrod_leapfrog.o: schrod_leapfrog.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

schrod_splitstep.o: schrod_splitstep.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

schrod_splitstep_image_potential.o: schrod_splitstep_image_potential.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

sph.o: sph.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

stable_fluids.o: stable_fluids.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

summation_stuff.o: summation_stuff.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

texture_data.o: texture_data.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

texture_data_transfers_stuff.o: texture_data_transfers_stuff.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

waves.o: waves.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

gl_wrappers.o: ${C_SOURCES}
	${C_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o *.bin *.dat ${TARGET}
