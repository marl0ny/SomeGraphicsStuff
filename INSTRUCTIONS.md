## Prerequisites
### OpenGL:
Currently the OpenGL programs only compile under Linux. The required libraries are [GLES](https://www.khronos.org/opengles/), [GLEW](http://glew.sourceforge.net/), and [GLFW](https://www.glfw.org/).
For the Spherical Harmonics project, you must also have Git, Python3 with the Sympy library and the Boost libraries installed.
### WebGL:
The WebGL programs require a modern web browser that supports WebGL. These include Chrome, Firefox, Edge, etc.

## Usage
### OpenGL:
The code for the OpenGL programs are found in subdirectories of the OpenGL directory. Enter into one of these subdirectories and type `make` to build the program, then run the subsequent executable file.
### WebGL:
To use the WebGL programs, enter any subdirectory of the WebGL directory and open the index.html file with a web browser. If you have python3.6 or later, you can also run `python -m run` to start the program.

Once you have the program running, use the mouse to rotate the various renders.
