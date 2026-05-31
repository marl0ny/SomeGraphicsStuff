##############################################################################

# Make sure to source <emcc_location>/emsdk/emsdk_env.sh first!

# WHEN CREATING A NEW DIRECTORY

# 1.) Add ./new_dir to the INCLUDE path.

# 2.) Define NEW_SOURCES = ./new_dir/file1.cpp ./new_dir/file2.cpp ...
#.    If files were moved to ./new_dir, REMEMBER TO REMOVE THEM from the
#		other rulesets.

# 3.) Define the objects that correspond to the sources above, BUT PLACE IT
#.    IN THE TOP LEVEL DIRECTORY, so do
#.    NEW_OBJECTS = ./file1.obj ./file2.obj ...
#.    If files were moved to ./new_dir, REMEMBER TO REMOVE THEM from the
#		other rulesets.

# 4.) Define the recipe for creating NEW_OBJECTS from NEW_SOURCES

# 5.) MAKE SURE to add NEW_OBJECTS to OBJECTS

# 6.) Remember to add ./new_dir path to .clangd. Also might be necessary
#.    to add ../new_dir

##############################################################################

ifeq ($(shell uname),Darwin)
INCLUDE =  -I${PWD} -I${PWD}/gl_wrappers -I${PWD}/glfw_wrappers \
	-I/opt/homebrew/include -I${PWD}/visualization_3d -I${PWD}/matrix \
	 -I${PWD}/parse -I${PWD}/user_edit
LIBS = -ldl -L/opt/homebrew/lib -lglfw\
       -framework CoreVideo -framework OpenGL -framework IOKit \
       -framework Cocoa -framework Carbon
else
INCLUDE = -I${PWD} -I${PWD}/gl_wrappers -I${PWD}/glfw_wrappers -I${PWD}/visualization_3d \
	 -I${PWD}/matrix -I${PWD}/parse -I${PWD}/user_edit
LIBS = -lm -lGL -lGLEW -lglfw
endif

##############################################################################

# TARGET = ${PWD}/program
# FLAGS = -O2 -g
# FLAGS2 = 
# CPP_COMPILE = clang++ -std=c++17

TARGET = main.js
FLAGS = -std=c++17 -Os -O2
FLAGS2 = -v -s WASM=2 -s USE_GLFW=3 -s MAX_WEBGL_VERSION=2 \
	-s ALLOW_MEMORY_GROWTH=1 -s LLD_REPORT_UNDEFINED \
	-lembind --embed-file shaders
CPP_COMPILE = emcc
LIBS = 

##############################################################################

DATA_DEPENDENCIES = parameters.json
GENERATION_SCRIPTS = make_parameter_files.py
GENERATED_DEPENDENCIES = parameters.hpp sliders.js
DEPENDENCIES = ${DATA_DEPENDENCIES} ${GENERATED_DEPENDENCIES}

all: ${TARGET}

TOP_LEVEL_SOURCES = main.cpp simulation.cpp
VISUALIZATION_3D_SOURCES = ./visualization_3d/volume_render.cpp \
	./visualization_3d/planar_slice.cpp ./visualization_3d/line_arrows3d.cpp \
	./visualization_3d/conical_arrow3d.cpp ./visualization_3d/cube_outline.cpp \
	./visualization_3d/cursor_outline3d.cpp
GL_WRAPPER_SOURCES = ./gl_wrappers/gl_wrappers.cpp
GLFW_WRAPPER_SOURCES = ./glfw_wrappers/glfw_window.cpp ./glfw_wrappers/interactor.cpp
MATRIX_SOURCES = ./matrix/matrix.cpp
PARSE_SOURCES = ./parse/parse.cpp
USER_EDIT_SOURCES = ./user_edit/user_edit_glsl.cpp

TOP_LEVEL_OBJECTS = main.o simulation.o
VISUALIZATION_3D_OBJECTS = ./volume_render.o \
	./planar_slice.o ./line_arrows3d.o \
	./conical_arrow3d.o ./cube_outline.o \
	./cursor_outline3d.o
GL_WRAPPER_OBJECTS = ./gl_wrappers.o
GLFW_WRAPPER_OBJECTS = ./glfw_window.o ./interactor.o
MATRIX_OBJECTS = ./matrix.o
PARSE_OBJECTS = ./parse.o
USER_EDIT_OBJECTS = ./user_edit_glsl.o

OBJECTS = ${TOP_LEVEL_OBJECTS} ${VISUALIZATION_3D_OBJECTS} \
	${GL_WRAPPER_OBJECTS} ${GLFW_WRAPPER_OBJECTS} ${MATRIX_OBJECTS} \
	${PARSE_OBJECTS} ${USER_EDIT_OBJECTS}

SHADERS := $(wildcard ./shaders/**/*.frag) $(wildcard ./shaders/**/*.vert)

${TARGET}: ${OBJECTS} ${SHADERS}
	${CPP_COMPILE} ${FLAGS} -o $@ ${OBJECTS} ${LIBS} ${FLAGS2}

${TOP_LEVEL_OBJECTS}: ${TOP_LEVEL_SOURCES} ${DEPENDENCIES}
	${CPP_COMPILE} ${FLAGS} -c ${TOP_LEVEL_SOURCES} ${INCLUDE}

${VISUALIZATION_3D_OBJECTS}: ${VISUALIZATION_3D_SOURCES} ${DEPENDENCIES}
	${CPP_COMPILE} ${FLAGS} -c ${VISUALIZATION_3D_SOURCES} ${INCLUDE}

${GL_WRAPPER_OBJECTS}: ${GL_WRAPPER_SOURCES} ${DEPENDENCIES}
	${CPP_COMPILE} ${FLAGS} -c ${GL_WRAPPER_SOURCES} ${INCLUDE}

${GLFW_WRAPPER_OBJECTS}: ${GLFW_WRAPPER_SOURCES} ${DEPENDENCIES}
	${CPP_COMPILE} ${FLAGS} -c ${GLFW_WRAPPER_SOURCES} ${INCLUDE}

${MATRIX_OBJECTS}: ${MATRIX_SOURCES} ${DEPENDENCIES}
	${CPP_COMPILE} ${FLAGS} -c ${MATRIX_SOURCES} ${INCLUDE}

${PARSE_OBJECTS}: ${PARSE_SOURCES} ${DEPENDENCIES}
	${CPP_COMPILE} ${FLAGS} -c ${PARSE_SOURCES} ${INCLUDE}

${USER_EDIT_OBJECTS}: ${USER_EDIT_SOURCES} ${DEPENDENCIES}
	${CPP_COMPILE} ${FLAGS} -c ${USER_EDIT_SOURCES} ${INCLUDE}

${GENERATED_DEPENDENCIES}: ${DATA_DEPENDENCIES} ${GENERATION_SCRIPTS}
	python3 make_parameter_files.py

clean_obj:
	rm -f *.o

clean:
	rm -f *.o ${TARGET} *.wasm *.js
