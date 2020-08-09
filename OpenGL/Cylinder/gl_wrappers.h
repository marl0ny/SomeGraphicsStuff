#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifndef _GL_WRAPPERS
#define _GL_WRAPPERS


/*
* Compile the shader, given the reference to the shader
* and the shader source code.
*/
void compile_shader(GLuint shader_ref, const char *shader_source);


/* 
* Create and compile the vertex shader. 
*/
GLuint make_vertex_shader(const char *v_source);


/* 
* Create and compile the fragment shader. 
*/
GLuint make_fragment_shader(const char *f_source);


/*
* Make the program with references to the shader.
*/
GLuint make_program(GLuint vs_ref, GLuint fs_ref);


/*
* Print vendor information.
*/
void print_vendor_information();


#endif
