#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "gl_wrappers.h"
#include <stdlib.h>


GLuint make_program(GLuint vs_ref, GLuint fs_ref) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vs_ref);
    glAttachShader(program, fs_ref);
    glLinkProgram(program);
    glUseProgram(program);
    return program;
}


void init_glew() {
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error.\n");
        exit(1);
    }
}


GLFWwindow *init_window(int width, int height) {
    if (glfwInit() != GL_TRUE) {
        fprintf(stderr, "Unable to create glfw window.\n");
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    GLFWwindow *window = glfwCreateWindow(width, height, "OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);
    return window;
}


GLuint make_vertex_array_object() {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    return vao;
}

GLuint make_vertex_buffer_object(float *data, size_t data_size) {
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 data_size,
                 data, GL_STATIC_DRAW);
    return vbo;
}


GLuint make_frame_buffer_object() {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    return fbo;
}

GLuint make_render_buffer_object(size_t depth_stencil, 
                                 int width, int height) {
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, depth_stencil, width, height);
    return rbo;
}


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


GLuint make_texture(void *image, size_t image_w, size_t image_h) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
   glTexImage2D(GL_TEXTURE_2D, 0,
                GL_RGBA32F,
                 image_w, image_h, 0, GL_RG,
                 GL_FLOAT, 
                 image);
    glGenerateMipmap(GL_TEXTURE_2D);
    return texture;
}


GLuint init_attrib(GLuint program, char *attrib_name,
                   size_t number_of_elements, size_t vertex_size, size_t offset) {
    GLint attrib = glGetAttribLocation(program, attrib_name);
    glEnableVertexAttribArray(attrib);
    // TODO: check if anything wrong occurs with this.
    size_t offset_in_bytes = offset*sizeof(float);
    glVertexAttribPointer(attrib, number_of_elements, 
                          GL_FLOAT, GL_FALSE, 
                          vertex_size*sizeof(float), 
                          (const void *)offset_in_bytes);
    return attrib;
}


void check_framebuffer() {
    char *status;
    int val;
    if ((val = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
        switch(val) {
            case GL_FRAMEBUFFER_UNDEFINED:
            status = "GL_FRAMEBUFFER_UNDEFINED";
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            status = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            status = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            status = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            status = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
            case GL_FRAMEBUFFER_UNSUPPORTED:
            status = "GL_FRAMEBUFFER_UNSUPPORTED";
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            status = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            default:
            status = "unknown";
        }
        fprintf(stderr, "Framebuffer incomplete: %x %s\n", val, status);
        exit(1);
    }
}


struct FrameTextureRender make_frame(int screen_w, int screen_h) {
    struct FrameTextureRender frame;
    frame.texture = make_texture(NULL, screen_w, screen_h);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    frame.fbo = make_frame_buffer_object();
    frame.rbo = make_render_buffer_object(GL_DEPTH32F_STENCIL8, screen_w, screen_h);
    frame.rbo = make_render_buffer_object(GL_DEPTH_COMPONENT32F, screen_w, screen_h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                              GL_RENDERBUFFER, frame.rbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                           GL_TEXTURE_2D, frame.texture, 0);
    check_framebuffer();
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return frame;
}


void print_vendor_information() {
    // Reference:
    // https://stackoverflow.com/a/42249529
    // answer by OutOfBound 
    // (https://stackoverflow.com/users/7042011/)
    // question
    // (https://stackoverflow.com/questions/42245870/)
    // by John Smith
    // (https://stackoverflow.com/users/7242130/)
    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    printf("%s\n", vendor);
    printf("%s\n", renderer);
}
