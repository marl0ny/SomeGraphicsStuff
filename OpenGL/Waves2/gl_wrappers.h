#define GL_SILENCE_DEPRECATION
// #define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>
#ifndef _GL_WRAPPERS
#define _GL_WRAPPERS


GLFWwindow *init_window(int width, int height);

GLuint make_program(GLuint vs_ref, GLuint fs_ref);

void compile_shader(GLuint shader_ref, const char *shader_source);

GLuint make_vertex_shader(const char *v_source);

GLuint make_fragment_shader(const char *f_source);

char *get_file_contents(const char *filename);

GLuint get_shader(const char *shader_loc, GLuint shader_type);

GLuint get_tex();

GLuint make_texture(uint8_t *image, size_t image_w, size_t image_h);

GLuint make_float_texture(float *image, size_t image_w, size_t image_h);

void set_int_uniform(const char *name, int val);

void set_float_uniform(const char *name, float val);

void set_vec2_uniform(const char *name, float v0, float v1);

void set_vec3_uniform(const char *name, float v0, float v1, float v2);

void set_vec4_uniform(const char *name,
                      float v0, float v1, float v2, float v3);

int new_ubyte_quad(int width, int height);

int new_float_quad(int width, int height);

void bind_quad(int quad_id, GLuint program);

void unbind();

void draw();

void get_texture_array(int x0, int y0, int width, int height, 
                       int texture_type, void *array);

void substitute_array(int width, int height, int texture_type, void *array);


#endif