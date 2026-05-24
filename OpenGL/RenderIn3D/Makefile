CPP_SOURCES = simulation.cpp \
	volume_render.cpp planar_slice.cpp line_arrows3d.cpp conical_arrow3d.cpp cube_outline.cpp cursor_outline3d.cpp \
	main.cpp \
	interactor.cpp gl_wrappers.cpp glfw_window.cpp parse.cpp user_edit_glsl.cpp matrix.cpp
OBJECTS = simulation.o \
	volume_render.o planar_slice.o line_arrows3d.o conical_arrow3d.o cube_outline.o cursor_outline3d.o \
	main.o \
	interactor.o gl_wrappers.o glfw_window.o parse.o user_edit_glsl.o matrix.o
SHADERS = ./shaders/gradient/gradient3d.frag ./shaders/slices/planar-slice.vert ./shaders/slices/quartered-square-outline.vert ./shaders/vol-render/sample-display.frag ./shaders/vol-render/display.frag ./shaders/vol-render/sample.frag ./shaders/vol-render/color-height-map1.frag ./shaders/vol-render/domain-coloring.frag ./shaders/vol-render/gray-scale.frag ./shaders/vol-render/modify-boundaries.frag ./shaders/vol-render/uniform-color-scale.frag ./shaders/vol-render/display.vert ./shaders/util/copy.frag ./shaders/util/scale.frag ./shaders/util/zero-boundaries-3d.frag ./shaders/util/not-used/surface/single-color.frag ./shaders/util/not-used/surface/mag-color-map.frag ./shaders/util/not-used/surface/surface.vert ./shaders/util/not-used/surface/domain-coloring.frag ./shaders/util/not-used/domain-color.frag ./shaders/util/not-used/mag-color-map.frag ./shaders/util/not-used/flip-horizontal.frag ./shaders/util/not-used/sub-window.frag ./shaders/util/uniform-color.frag ./shaders/util/blur.frag ./shaders/util/slice-of-3d.frag ./shaders/cube-outline/cube-outline.vert ./shaders/arrows/line-arrows3d.vert ./shaders/arrows/conical-arrows3d.vert

SHELL = /bin/bash
FLAGS = -O2 -g
C_COMPILE = clang
CPP_COMPILE = clang++ -std=c++17
LINKER = /usr/bin/ld

ifeq ($(shell uname),Darwin)
INCLUDE =  -I${PWD} -I${PWD}/gl_wrappers -I/opt/homebrew/include
LIBS = -ldl -L/opt/homebrew/lib -lglfw\
       -framework CoreVideo -framework OpenGL -framework IOKit\
       -framework Cocoa -framework Carbon
else
INCLUDE =  -I${PWD} -I${PWD}/gl_wrappers
LIBS = -lm -lGL -lGLEW -lglfw
endif

# Make sure to source <emcc_location>/emsdk/emsdk_env.sh first!
WEB_TARGET = main.js

TARGET = ${PWD}/program
DATA_DEPENDENCIES = parameters.json
GENERATION_SCRIPTS = make_parameter_files.py
GENERATED_DEPENDENCIES = parameters.hpp
C_SOURCES =
SOURCES = ${C_SOURCES} ${CPP_SOURCES}
# SHADERS = ./shaders/*


all: ${TARGET}

${TARGET}: ${OBJECTS}
	${CPP_COMPILE} ${FLAGS} -o $@ ${OBJECTS} ${LIBS}

${WEB_TARGET}: ${SOURCES} ${GENERATED_DEPENDENCIES} ${SHADERS}
	emcc -lembind -o $@ ${SOURCES} ${INCLUDE} -std=c++17 -O3 -v -s WASM=2 -s USE_GLFW=3 -s MAX_WEBGL_VERSION=2 \
	-s ALLOW_MEMORY_GROWTH=1 -s LLD_REPORT_UNDEFINED --embed-file shaders

${OBJECTS}: ${CPP_SOURCES} ${GENERATED_DEPENDENCIES}
	${CPP_COMPILE} ${FLAGS} -c ${CPP_SOURCES} ${INCLUDE}

${GENERATED_DEPENDENCIES}: ${DATA_DEPENDENCIES} ${GENERATION_SCRIPTS}
	python3 make_parameter_files.py

clean:
	rm -f *.o ${TARGET} *.wasm *.js
