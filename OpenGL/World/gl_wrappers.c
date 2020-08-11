#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "gl_wrappers.h"
#include <stdlib.h>


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


GLuint make_program(GLuint vs_ref, GLuint fs_ref) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vs_ref);
    glAttachShader(program, fs_ref);
    // glBindFragDataLocation(program, 0, "colour"); 
    glLinkProgram(program);
    glUseProgram(program);
    return program;
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
