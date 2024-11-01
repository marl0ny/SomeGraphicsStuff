SHELL = /bin/bash


IMGUI_DIR = ${PWD}/imgui

INCLUDE = -I${PWD} -I${PWD}/gl_wrappers -I${IMGUI_DIR} -I${IMGUI_DIR}/backends
LIBS = -lm -lGL -lGLEW -lglfw -pthread -lpng

# For MacOS you must first install the relavent OpenGL and GLFW libraries.
# Depending on which package manager you used, as well as the age and 
# type of your system, the actual location of these libraries will vary.
# You must first determine the actual location of the developement directories
# for your own system, and then edit the INCLUDE and LIBS 
# variables accordingly.
ifeq ($(shell uname),Darwin)
INCLUDE =  -I${PWD} -I${PWD}/gl_wrappers -I/opt/homebrew/include\
					 -I/usr/local/include -I${IMGUI_DIR} -I${IMGUI_DIR}/backends
LIBS = -ldl -L/opt/homebrew/lib -L/usr/local/include -lglfw\
       -framework CoreVideo -framework OpenGL -framework IOKit\
       -framework Cocoa -framework Carbon -framework Accelerate\
	   -lpng
endif


# TARGET = ${PWD}/program
# FLAGS = -O2 -g -ffast-math -Wall # -Wno-everything
# FLAGS_NO_OPT = -O0 -g -Wall 
# # FLAGS = -O0 -g -Wall
# C_COMPILE = clang 
# CPP_COMPILE = clang++ -std=c++11

TARGET = main.js
# CPP_COMPILE = EMCC_DEBUG=1 emcc
# C_COMPILE = EMCC_DEBUG=1 emcc
CPP_COMPILE = emcc
C_COMPILE = emcc
FLAGS = -O3 -v -s WASM=1 -s USE_GLFW=3 -s FULL_ES3=1 \
	-s TOTAL_MEMORY=100MB --embed-file shaders # -pthread -sPTHREAD_POOL_SIZE=12 
FLAGS_NO_OPT = ${FLAGS}
LIBS =
INCLUDE = -I${IMGUI_DIR} -I${IMGUI_DIR}/backends

C_SOURCES = ${PWD}/gl_wrappers/gl_wrappers.c frames_stacks.c bin_ops.c \
             unary_ops.c fft.c bitmap.c summation.c write_to_png.c \
						 read_from_png.c serialize.c
C_OBJECTS = gl_wrappers.o frames_stacks.o bin_ops.o unary_ops.o fft.o \
			 bitmap.o summation.o write_to_png.o read_from_png.o serialize.o
CPP_SOURCES = render.cpp interactor.cpp quaternions.cpp volume_render.cpp parse_util.cpp \
							complex_func.cpp dirac_leapfrog.cpp draw_texture_data.cpp \
							vector_field_view_2d.cpp vector_field_view_3d.cpp \
							isf_splitstep.cpp main.cpp mol_dynamics.cpp mol_dynamics_mt.cpp \
							n_body.cpp poisson.cpp reaction_diffusion.cpp \
							reduction_to_1d.cpp schrod_leapfrog.cpp schrod_splitstep.cpp \
							schrod_splitstep_image_potential.cpp stable_fluids.cpp \
							summation_stuff.cpp texture_data.cpp fft_omp.cpp \
							texture_data_transfers_stuff.cpp waves.cpp sph.cpp \
							schrod_leapfrog_3d.cpp schrod_splitstep_3d.cpp pauli_leapfrog_3d.cpp \
							dirac_splitstep_3d.cpp electrodynamics_3d.cpp free_space_position.cpp
CPP_OBJECTS = render.o interactor.o quaternions.o parse_util.o \
					texture_data.o draw_texture_data.o \
					vector_field_view_2d.o vector_field_view_3d.o volume_render.o \
					texture_data_transfers_stuff.o summation_stuff.o \
					reduction_to_1d.o complex_func.o waves.o \
					poisson.o reaction_diffusion.o stable_fluids.o \
					n_body.o mol_dynamics.o mol_dynamics_mt.o sph.o \
					schrod_splitstep_image_potential.o schrod_leapfrog.o\
					schrod_splitstep.o isf_splitstep.o \
					dirac_leapfrog.o fft_omp.o  schrod_leapfrog_3d.o pauli_leapfrog_3d.o \
					schrod_splitstep_3d.o dirac_splitstep_3d.o \
					electrodynamics_3d.o free_space_position.o main.o

IMGUI_DIR = ${PWD}/imgui
IMGUI_SOURCES = ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp \
                ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp \
                ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp \
                ${IMGUI_DIR}/imgui_tables.cpp \
                ${IMGUI_DIR}/imgui_widgets.cpp
IMGUI_OBJECTS = imgui.o imgui_draw.o imgui_impl_glfw.o imgui_impl_opengl3.o \
                imgui_tables.o imgui_widgets.o

OBJECTS = ${IMGUI_OBJECTS} ${C_OBJECTS} ${CPP_OBJECTS}

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${CPP_COMPILE} ${FLAGS} -o $@ $^ ${LIBS} ${INCLUDE}

${IMGUI_SOURCES}:
	wget "https://github.com/ocornut/imgui/archive/refs/heads/master.zip"
	unzip master.zip
	mv ./imgui-master ./imgui

${IMGUI_OBJECTS}: ${IMGUI_SOURCES}
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}


# for e in $(find *.cpp); \
do printf $e | sed 's/\.cpp/\.o:/g'; echo ' '$e; \
printf '\t${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}' ; echo; echo; done


complex_func.o: complex_func.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

dirac_leapfrog.o: dirac_leapfrog.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

dirac_splitstep_3d.o: dirac_splitstep_3d.cpp
	${CPP_COMPILE} ${FLAGS_NO_OPT} -c $^ ${INCLUDE}

draw_texture_data.o: draw_texture_data.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

electrodynamics_3d.o: electrodynamics_3d.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

fft_omp.o: fft_omp.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

free_space_position.o: free_space_position.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

interactor.o: interactor.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

isf_splitstep.o: isf_splitstep.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

main.o: main.cpp
	${CPP_COMPILE} ${FLAGS_NO_OPT} -c $^ ${INCLUDE}

mol_dynamics.o: mol_dynamics.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

mol_dynamics_mt.o: mol_dynamics_mt.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

n_body.o: n_body.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

parse_util.o: parse_util.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

pauli_leapfrog_3d.o: pauli_leapfrog_3d.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

poisson.o: poisson.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

quaternions.o: quaternions.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

reaction_diffusion.o: reaction_diffusion.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

reduction_to_1d.o: reduction_to_1d.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

render.o: render.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

schrod_leapfrog_3d.o: schrod_leapfrog_3d.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

schrod_leapfrog.o: schrod_leapfrog.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

schrod_splitstep.o: schrod_splitstep.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

schrod_splitstep_3d.o: schrod_splitstep_3d.cpp
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

vector_field_view_2d.o: vector_field_view_2d.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

vector_field_view_3d.o: vector_field_view_3d.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

volume_render.o: volume_render.cpp
	${CPP_COMPILE} ${FLAGS_NO_OPT} -c $^ ${INCLUDE}

waves.o: waves.cpp
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

gl_wrappers.o: ${C_SOURCES}
	${C_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

clean:
	rm -f *.o *.bin *.dat ${TARGET}
