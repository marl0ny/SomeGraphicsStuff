#include "gl_wrappers.h"
#include <stdlib.h>
#include <stdio.h>


GLFWwindow *init_window(int width, int height) {
    if (glfwInit() != GL_TRUE) {
        fprintf(stderr, "Unable to create glfw window.\n");
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    GLFWwindow *window = glfwCreateWindow(width/2, height/2, "Waves!", 
                                          NULL, NULL);
    if (!window) {
        fprintf(stderr, "Unable to create glfw window.\n");
        exit(1);
    }
    glfwMakeContextCurrent(window);
    return window;
}

GLuint make_program(GLuint vs_ref, GLuint fs_ref) {
    GLuint program = glCreateProgram();
    if (program == 0) {
        fprintf(stderr, "Unable to create program.\n");
    }
    // std::cout << program << ", " << glGetError() << std::endl;
    glAttachShader(program, vs_ref);
    glAttachShader(program, fs_ref);
    glLinkProgram(program);
    GLint status;
    char buf[512];
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    glGetProgramInfoLog(program, 512, NULL, buf);
    if (status != GL_TRUE) {
        fprintf(stderr, "%s\n%s", "Failed to link program:", buf);
    }
    glUseProgram(program);
    return program;
}

/*void init_glew() {
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error.\n");
        exit(1);
    }
}*/

void compile_shader(GLuint shader_ref, const char *shader_source) {
    char buf[512];
    glShaderSource(shader_ref, 1, &shader_source, NULL);
    glCompileShader(shader_ref);
    GLint status;
    glGetShaderiv(shader_ref, GL_COMPILE_STATUS, &status);
    glGetShaderInfoLog(shader_ref, 512, NULL, buf);
    if (status == GL_TRUE) {
        if (buf[0] != '\0') {
            fprintf(stdout, "%s", buf);
        }
    } else {
        fprintf(stderr, "%s\n%s", "Shader compilation failed:", buf);
    }
}

GLuint make_vertex_shader(const char *v_source) {
    GLuint vs_ref = glCreateShader(GL_VERTEX_SHADER);
    if (vs_ref == 0) {
        fprintf(stderr, "unable to "
                "create vertex shader (error code %d).\n",
                glGetError());
        exit(1);
    }
    compile_shader(vs_ref, v_source);
    return vs_ref;
}

GLuint make_fragment_shader(const char *f_source) {
    GLuint fs_ref = glCreateShader(GL_FRAGMENT_SHADER);
    if (fs_ref == 0) {
        fprintf(stderr, "Error: unable to "
                "create fragment shader (error code %d).\n",
                glGetError());
        exit(1);
    }
    compile_shader(fs_ref, f_source);
    return fs_ref;
}

/*
* Given a filename, get its file contents.
*/
char *get_file_contents(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        perror("fopen");
        fclose(f);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    int file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *file_buff = (char *)malloc(file_size + 1);
    if (file_buff == NULL) {
        perror("malloc");
        fclose(f);
        return NULL;
    }
    fread(file_buff, file_size, 1, f);
    file_buff[file_size] = '\0';
    fclose(f);
    return file_buff;
}


GLuint get_shader(const char *shader_loc, GLuint shader_type) {
    char *contents = get_file_contents(shader_loc);
    if (shader_type == GL_VERTEX_SHADER) {
        return make_vertex_shader(contents);
    } else {
        return make_fragment_shader(contents);
    }
    free(contents);
    return 0;
}

GLuint get_tex() {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    return texture;
}

void do_texture_paramerteri_and_mipmap() {
    glGenerateMipmap(GL_TEXTURE_2D);
    // Switching from glTextureParameteri to glTexParameteri actually gets
    // the third argument which has to do with the boundaries to be followed. 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void do_texture_paramerterf_and_mipmap() {
    glGenerateMipmap(GL_TEXTURE_2D);
    // Switching from glTextureParameteri to glTexParameteri actually gets
    // the third argument which has to do with the boundaries to be followed. 
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
} 

GLuint make_ubyte_texture(uint8_t *image, size_t image_w, size_t image_h) {
    GLuint texture = get_tex();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
                 image_w, image_h, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, image);
    do_texture_paramerteri_and_mipmap();
    return texture;
}

GLuint make_float_texture(float *image, size_t image_w, size_t image_h) {
    GLuint texture = get_tex();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
                 image_w, image_h, 0, GL_RGBA,
                 GL_FLOAT, image);
    do_texture_paramerterf_and_mipmap();
    return texture;
}

static const int MAX_FRAME_COUNT = 100;

struct Frame {
    int frame_type;
    GLuint program;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint fbo;
    GLuint texture;
};

static struct Frame frames[MAX_FRAME_COUNT];
static struct Frame *current_frame = NULL;
static int current_frame_id = 0;
static int total_frames = 0;
static float vertices[12] = {
    -1.0, -1.0, 0.0,
     -1.0, 1.0, 0.0, 
     1.0, 1.0, 0.0, 
     1.0, -1.0, 0.0};
static int elements[6] = {
    0, 1, 2, 0, 2, 3};

void quad_init_texture(int width, int height, int texture_type) {
    if (current_frame != NULL) {
        if (current_frame_id != 0) {
            glActiveTexture(GL_TEXTURE0 + current_frame_id);
            if (texture_type == GL_UNSIGNED_BYTE) {
                current_frame->texture = make_ubyte_texture(
                    NULL, width, height);
            } else if (texture_type == GL_FLOAT) {
                current_frame->texture = make_float_texture(
                    NULL, width, height);
            }
        }
    }
}

void quad_init_objects() {
    GLuint *vao_ptr = &current_frame->vao;
    GLuint *vbo_ptr = &current_frame->vbo;
    GLuint *ebo_ptr = &current_frame->ebo;
    GLuint *fbo_ptr = &current_frame->fbo;
    GLuint *texture_ptr = &current_frame->texture;
    glGenVertexArrays(1, vao_ptr);
    glBindVertexArray(*vao_ptr);
    glGenBuffers(1, vbo_ptr);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo_ptr);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), 
                    vertices, GL_STATIC_DRAW);
    glGenBuffers(1, ebo_ptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo_ptr);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements),
                 elements, GL_STATIC_DRAW);
    if (current_frame_id != 0) {
        glGenFramebuffers(1, fbo_ptr);
        glBindFramebuffer(GL_FRAMEBUFFER, *fbo_ptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                GL_TEXTURE_2D, *texture_ptr, 0);
    }
    // printf("%d, %d, %d, %d, %d\n", 
    //        *vao_ptr, *vbo_ptr, *ebo_ptr, *fbo_ptr, *texture_ptr);
}

void set_int_uniform(const char *name, int val) {
    if (current_frame != NULL) {
        GLuint program = current_frame->program;
        GLuint loc = glGetUniformLocation(program, name);
        glUniform1i(loc, val);
    }
}

void set_float_uniform(const char *name, float val) {
    if (current_frame != NULL) { 
        GLuint program = current_frame->program;
        GLuint loc = glGetUniformLocation(program, name);
        glUniform1f(loc, val);
    }
}

void set_vec2_uniform(const char *name, float v0, float v1) {
    if (current_frame != NULL) { 
        GLuint program = current_frame->program;
        GLuint loc = glGetUniformLocation(program, name);
        glUniform2f(loc, v0, v1);
    }
}

void set_vec3_uniform(const char *name, float v0, float v1, float v2) {
    if (current_frame != NULL) {
        GLuint program = current_frame->program;
        GLuint loc = glGetUniformLocation(program, name);
        glUniform3f(loc, v0, v1, v2);
    }
}

void set_vec4_uniform(const char *name,
                      float v0, float v1, float v2, float v3) {
    if (current_frame != NULL) {
        GLuint program = current_frame->program;
        GLuint loc = glGetUniformLocation(program, name);
        glUniform4f(loc, v0, v1, v2, v3);
    }
}

int new_ubyte_quad(int width, int height) {
    int quad_id = total_frames;
    total_frames += 1;
    current_frame = &frames[quad_id];
    current_frame_id = quad_id;
    quad_init_texture(width, height, GL_UNSIGNED_BYTE);
    quad_init_objects();
    unbind();
    return quad_id;
}

int new_float_quad(int width, int height) {
    int quad_id = total_frames;
    total_frames += 1;
    current_frame = &frames[quad_id];
    current_frame_id = quad_id;
    quad_init_texture(width, height, GL_FLOAT);
    quad_init_objects();
    unbind();
    return quad_id;
}

void bind_quad(int quad_id, GLuint program) {
    if (current_frame != NULL) {
        return;
    }
    current_frame_id = quad_id;
    current_frame = &frames[quad_id];
    current_frame->program = program;
    glUseProgram(program);
    glBindVertexArray(current_frame->vao);
    glBindBuffer(GL_ARRAY_BUFFER, current_frame->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, current_frame->ebo);
    if (current_frame_id != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, current_frame->fbo);
    }
    // glActiveTexture(GL_TEXTURE0 + quad_id);
    GLint attrib = glGetAttribLocation(current_frame->program,
                                       "position");
    glEnableVertexAttribArray(attrib);
    glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, 3*4, 0);
}

void unbind() {
    if (current_frame != NULL) {
        current_frame_id = -1;
        current_frame = NULL;
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
}

void draw() {
    if (current_frame == NULL) {
        return;
    } else if (current_frame->frame_type == 0) {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    } // else if () {}
}

void get_texture_array(int x0, int y0, int width, int height, 
                       int texture_type, void *array) {
    glActiveTexture(GL_TEXTURE0 + current_frame_id);
    glBindTexture(GL_TEXTURE_2D, current_frame->texture);
    if (texture_type == GL_UNSIGNED_BYTE) {
        return glReadPixels(x0, y0, width, height, GL_RGBA, 
                            GL_UNSIGNED_BYTE, array);
    } else if (texture_type == GL_FLOAT) {
        return glReadPixels(x0, y0, width, height, GL_RGBA, GL_FLOAT, array);
    }
}


void substitute_array(int width, int height, int texture_type, void *array) {
    if (current_frame != NULL) {
        glActiveTexture(GL_TEXTURE0 + current_frame_id);
        glBindTexture(GL_TEXTURE_2D, current_frame->texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                        width, height, GL_RGBA, texture_type,
                        array);
        glActiveTexture(GL_TEXTURE0);
    }
}

