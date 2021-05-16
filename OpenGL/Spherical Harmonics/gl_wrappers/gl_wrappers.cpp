#include "gl_wrappers.h"

namespace gl_wrappers {

    GLFWwindow *init_window(int width, int height) {
        if (glfwInit() != GL_TRUE) {
            fprintf(stderr, "Unable to create glfw window.\n");
            exit(1);
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        #ifndef __EMSCRIPTEN__
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
        #endif
        GLFWwindow *window = glfwCreateWindow(width, height, "GUI", NULL, NULL);
        glfwMakeContextCurrent(window);
        return window;
    }

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

    GLuint get_shader(const char *shader_loc, GLuint shader_type) {
        std::ifstream shader_stream{std::string(shader_loc)};
        if (!shader_stream) {
            std::fprintf(stderr, "Unable to open %s.", shader_loc);
            return 0;
        }
        std::string shader_source(10001, '\0');
        shader_stream.readsome((char *)shader_source.c_str(), 10000);
        if (shader_type == GL_VERTEX_SHADER) {
            return gl_wrappers::make_vertex_shader(shader_source.c_str());
        } else {
            return gl_wrappers::make_fragment_shader(shader_source.c_str());
        }
        return 0;
    }

    GLuint get_tex() {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        return texture;
    }

    void do_texture_paramertiri_and_mipmap() {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        #ifdef __EMSCRIPTEN__
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        #else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        GL_NEAREST_MIPMAP_NEAREST);
        #endif
    }

    GLuint make_texture(uint8_t *image, size_t image_w, size_t image_h) {
        GLuint texture = get_tex();
        #ifdef __EMSCRIPTEN__
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                    image_w, image_h, 0, GL_RGBA,
                    GL_UNSIGNED_BYTE, image);
        #else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                    image_w, image_h, 0, GL_RGBA,
                    GL_UNSIGNED_BYTE, image);
        #endif
        do_texture_paramertiri_and_mipmap();
        return texture;
    }

    GLuint make_float_texture(float *image, size_t image_w, size_t image_h) {
        GLuint texture = get_tex();
        #ifdef __EMSCRIPTEN__
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                    image_w, image_h, 0, GL_RGBA,
                    GL_FLOAT, image);
        #else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
                    image_w, image_h, 0, GL_RGBA,
                    GL_FLOAT, image);
        #endif
        do_texture_paramertiri_and_mipmap();
        return texture;
    }

    void unbind() {
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

}