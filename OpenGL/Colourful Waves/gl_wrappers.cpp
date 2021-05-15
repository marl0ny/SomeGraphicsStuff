#include "gl_wrappers.h"

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
        return make_vertex_shader(shader_source.c_str());
    } else {
        return make_fragment_shader(shader_source.c_str());
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
    // Switching from glTextureParameteri to glTexParameteri actually gets
    // the third argument which has to do with the boundaries to be followed. 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
} 

GLuint make_texture(uint8_t *image, size_t image_w, size_t image_h) {
    GLuint texture = get_tex();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
                image_w, image_h, 0, GL_RGB,
                GL_UNSIGNED_BYTE, image);
    do_texture_paramertiri_and_mipmap();
    return texture;
}

GLuint make_float_texture(float *image, size_t image_w, size_t image_h) {
    GLuint texture = get_tex();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
                 image_w, image_h, 0, GL_RGBA,
                 GL_FLOAT, image);
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

int Frame::total_frames = 0;

int Frame::get_value() const {
    return frame_number;
}

int Frame::get_tex_unit() const {
    return GL_TEXTURE0 + get_value();
}

Quad:: Quad() {}

void Quad::init_texture(int width, int height, int texture_type) {
    if (frame_number != 0) {
        glActiveTexture(GL_TEXTURE0 + frame_number);
        if (texture_type == GL_UNSIGNED_BYTE) {
            texture = make_texture(nullptr, width, height);
        } else if (texture_type == GL_FLOAT) {
            texture = make_float_texture(nullptr, width, height);   
        }
        glBindTexture(GL_TEXTURE_2D, texture);
    }
}

void Quad::init_objects() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float vertices[12] = {
        1.0, 1.0, 0.0, 1.0, -1.0, 0.0, 
        -1.0, -1.0, 0.0, -1.0, 1.0, 0.0};
    int elements[6] = {
        0, 2, 3, 0, 1, 2};
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), 
                    vertices, GL_STATIC_DRAW);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements),
                    elements, GL_STATIC_DRAW);
    if (frame_number != 0) {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                GL_TEXTURE_2D, texture, 0);
    }
    set_vertex_name(vertex_name);
}

void Quad::bind() {
    glBindVertexArray(vao);
    if (frame_number != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    set_vertex_name(vertex_name);
}

void Quad::set_vertex_name(const std::string &name) {
    GLint attrib = glGetAttribLocation(program, name.c_str());
    glEnableVertexAttribArray(attrib);
    glVertexAttribPointer(attrib, 3, GL_FLOAT, false, 3*4, 0);
}

void Quad::set_vertex_name(const char *name) {
    GLint attrib = glGetAttribLocation(program, name);
    glEnableVertexAttribArray(attrib);
    glVertexAttribPointer(attrib, 3, GL_FLOAT, false, 3*4, 0);
}

void Quad::set_program(GLuint program) {
    this->program = program;
    glUseProgram(program);
}

 Quad Quad::make_frame(int width, int height, 
                       const std::string &vertex_name) {
    Quad::total_frames += 1;
    Quad quad = Quad();
    for (int i = 0; i < 15; i++) {
        quad.vertex_name[i] = vertex_name[i];
    }
    quad.frame_number = Quad::total_frames - 1;
    quad.init_texture(width, height, GL_UNSIGNED_BYTE);
    quad.init_objects();
    quad.set_vertex_name(vertex_name);
    unbind();
    return quad;
}

Quad Quad::make_float_frame(int width, int height, 
                            const std::string &vertex_name) {
    Quad::total_frames += 1;
    Quad quad = Quad();
    for (int i = 0; i < 15; i++) {
        quad.vertex_name[i] = vertex_name[i];
    }
    quad.frame_number = Quad::total_frames - 1;
    quad.init_texture(width, height, GL_FLOAT);
    quad.init_objects();
    quad.set_vertex_name(vertex_name);
    unbind();
    return quad;
}

void Quad::set_int_uniform(const char *name, int val) {
    GLuint loc = glGetUniformLocation(program, name);
    glUniform1i(loc, val);
}

void Quad::set_int_uniform(const std::string &name, int val) {
    GLuint loc = glGetUniformLocation(program, name.c_str());
    glUniform1i(loc, val);
}

void Quad::set_int_uniforms(const std::map<std::string, int> &uniforms) {
    for (const auto &uniform: uniforms) {
        set_int_uniform(uniform.first, uniform.second);
    }

}

void Quad::set_float_uniform(const char *name, float val) {
    GLuint loc = glGetUniformLocation(program, name);
    glUniform1f(loc, val);
}

void Quad::set_float_uniform(const std::string &name, float val) {
    GLuint loc = glGetUniformLocation(program, name.c_str());
    glUniform1f(loc, val);
}

void Quad::set_float_uniforms(const std::map<std::string, double> &uniforms) {
    for (const auto &uniform: uniforms) {
        set_float_uniform(uniform.first, uniform.second);
    }
}

int Quad::get_texture() const {
    return texture;
}

void Quad::draw() const {
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
