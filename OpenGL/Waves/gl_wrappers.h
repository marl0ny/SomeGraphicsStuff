#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifndef _GL_WRAPPERS
#define _GL_WRAPPERS


/*
* Create a new GLFW window.
*/
GLFWwindow *init_window(int width, int height);


/*
* Initialize GLEW.
*/
void init_glew();


/*
* Make a single vertex array object.
*/
GLuint make_vertex_array_object();


/*
* Make a single vertex buffer object, taking
* a buffer and and its size as argument.
*/
GLuint make_vertex_buffer_object(float *data, size_t data_size);


/*
* Make a single frame buffer object.
*/
GLuint make_frame_buffer_object();

/*
* Make a single render buffer object,
* taking the depth stencil, width and height as
* arguments.
*/
GLuint make_render_buffer_object(size_t depth_stencil, int width, int height);

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
* Make a texture, given the image data and its dimensions.
*/
GLuint make_texture(void *image, size_t image_w, size_t image_h);


/*
* Initialize an attribute, taking the program reference, the attribute name,
* the number of elements that the attribute uses, the size of the vertex,
* and the offset of the vertex as argument. These values are in size of float.
*/
GLuint init_attrib(GLuint program, char *attrib_name,
                   size_t number_of_elements, size_t vertex_size, size_t offset);


/*
* Frame texture render struct.
*/
struct FrameTextureRender {
    GLuint texture; // texture
    GLuint fbo; // frame buffer object
    GLuint rbo; // render buffer object
    GLuint vao; // vertex array object
    GLuint vbo; // vertex buffer object
    GLuint ebo; // element buffer object
};


/*
* Make a new frame buffer with a render buffer and a blank texture attatched.
*/
struct FrameTextureRender make_frame(int screen_w, int screen_h);


/*
* Print vendor information.
*/
void print_vendor_information();


#endif
