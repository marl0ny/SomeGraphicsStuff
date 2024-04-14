#include "gl_wrappers.h"
#include <GLES3/gl3.h>
#include <GLES3/gl32.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_FRAME_COUNT 100


static const char QUAD_VERTEX_SHADER_SOURCE[] = ""
    "#VERSION_NUMBER_PLACEHOLDER\n"
    "\n"
    "#if __VERSION__ <= 120\n"
    "attribute vec3 position;\n"
    "varying vec2 UV;\n"
    "#else\n"
    "in vec3 position;\n"
    "out highp vec2 UV;\n"
    "#endif\n"
    "\n"
    "void main() {\n"
    "    gl_Position = vec4(position.xyz, 1.0);\n"
    "    UV = position.xy/2.0 + vec2(0.5, 0.5);\n"
    "}\n";

enum FRAME_TYPES {QUAD=0, GENERAL_SINGLE_2D_FRAME};

struct Frame {
    int frame_type;
    GLuint program;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint fbo;
    GLuint rbo;
    GLuint texture;
    // int vertex_count;
    // int element_count;
    // int draw_type;
};

struct MultiFrame {

};

static int s_quad_vertex_shader_ref = -1;
static struct Frame s_frames[MAX_FRAME_COUNT];
static struct Frame *s_current_frame = NULL;
static int s_current_frame_id = 0;
static int s_total_frames = 0;
static int s_err_msg_counter = 0;

static const float QUAD_VERTICES[12] = {
    -1.0, -1.0, 0.0,
    -1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, -1.0, 0.0};
static const int QUAD_ELEMENTS[6] = {
    0, 1, 2, 0, 2, 3};

static const char ERR_NO_FRAME_ACTIVE[] = "No frame active.\n";
static const char ERR_FRAME_ACTIVE[] = "Frame already active.\n";


void copy_tex_params(struct TextureParams *dst,
                     const struct TextureParams *src) {
    dst->format = src->format;
    dst->width = src->width;
    dst->height = src->height;
    dst->generate_mipmap = src->generate_mipmap;
    dst->wrap_s = src->wrap_s;
    dst->wrap_t = src->wrap_t;
    dst->mag_filter = src->mag_filter;
    dst->min_filter = src->min_filter;
}

GLFWwindow *init_window(int width, int height) {
    if (glfwInit() != GL_TRUE) {
        #ifndef __EMSCRIPTEN__
	fprintf(stderr, "%s\n", glfwGetError(NULL));
        fprintf(stderr, "Unable to create glfw window.\n");
        exit(1);
        #endif
    }
    #ifdef __EMSCRIPTEN__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #else
    #ifdef OLD_OPENGL_VERSION
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    #else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    #endif
    #ifndef __APPLE__
    GLFWwindow *window = glfwCreateWindow(width, height, "Window",
                                          NULL, NULL);
    #else
    // TODO: This assumes that all Apple devices use Retina display,
    // and that all Retina displays have the same pixel densities.
    // Find a better way to implement this.
    GLFWwindow *window = glfwCreateWindow(width/2, height/2, "Window",
                                          NULL, NULL);
    #endif
    if (!window) {
        #ifndef __EMSCRIPTEN__
	fprintf(stderr, "%x\n", glfwGetError(NULL)); 
        fprintf(stderr, "Unable to create glfw window.\n");
        exit(1);
        #endif
    }
    glfwMakeContextCurrent(window);
    return window;
}

void compile_shader(GLuint shader_ref, const char *shader_source) {
    char buf[512];
    const char version_number_placeholder[] = "#VERSION_NUMBER_PLACEHOLDER";
    int m = 0;
    for (; m < sizeof(version_number_placeholder)-1; m++) {
        if (!shader_source[m] ||
            shader_source[m] != version_number_placeholder[m]) {
            fprintf(stderr, "Shader invalid.");
        }
    }
    for ( ; shader_source[m] != '\n'; m++) {
        if (!shader_source[m] || shader_source[m] != ' ') {
            fprintf(stderr, "Shader invalid");
            return;
        }
    }
    int size = 0;
    for (int i = 0; shader_source[i]; i++, size++);
    #ifndef __EMSCRIPTEN__
    int minor_version = 0, major_version = 0;
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    // fprintf(stdout, "%d, %d\n", major_version, minor_version);
    #ifdef OLD_OPENGL_VERSION
    const char version_number[] = "#version 120\n";
    #else
    const char version_number[] = "#version 330\n";
    #endif
    #else
    const char version_number[] = "#version 300 es  \n";
    #endif
    char *mod_source = (char *)calloc(1 + size, sizeof(char));
    if (mod_source == NULL) {
        perror("Unable to allocate resources for shader initialization.");
        return;
    }
    int i = 0, k = 0;
    for (; shader_source[i] != '\n'; i++) {
        if (i < sizeof(version_number) - 1) {
            mod_source[k++] = version_number[i];
        }
    }
    for (; (mod_source[k] = shader_source[i]); i++, k++);
    const char *tmp = (const char *)mod_source;
    glShaderSource(shader_ref, 1, &tmp, NULL);
    free(mod_source);
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

GLuint make_geometry_shader(const char *g_source) {
    GLuint gs_ref = glCreateShader(GL_GEOMETRY_SHADER);
    if (gs_ref == 0) {
        fprintf(stderr, "Error: unable to "
                "create geometry  shader (error code %d).\n",
                glGetError());
        exit(1);
    }
    compile_shader(gs_ref, g_source);
    return gs_ref;
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
    GLuint shader;
    if (shader_type == GL_VERTEX_SHADER) {
        shader = make_vertex_shader(contents);
    } else {
        shader = make_fragment_shader(contents);
    }
    free(contents);
    return shader;
}

GLuint get_vertex_shader(const char *shader_loc) {
    return get_shader(shader_loc, GL_VERTEX_SHADER);
}

GLuint get_fragment_shader(const char *shader_loc) {
    return get_shader(shader_loc, GL_FRAGMENT_SHADER);
}

GLuint make_program(const char *path_vertex, const char *path_fragment) {
    if (s_quad_vertex_shader_ref < 0) {
        s_quad_vertex_shader_ref
            = make_vertex_shader(QUAD_VERTEX_SHADER_SOURCE);
    }
    fprintf(stdout, "Compiling %s.\n", path_vertex);
    GLuint vs_ref = get_shader(path_vertex, GL_VERTEX_SHADER);
    fprintf(stdout, "Compiling %s.\n", path_fragment);
    GLuint fs_ref = get_shader(path_fragment, GL_FRAGMENT_SHADER);
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
        fprintf(stderr, "%s\n%s\n", "Failed to link program:", buf);
    }
    glUseProgram(program);
    return program;
}

GLuint make_quad_program(const char *frag_shader_loc) {
    if (s_quad_vertex_shader_ref < 0) {
        s_quad_vertex_shader_ref
            = make_vertex_shader(QUAD_VERTEX_SHADER_SOURCE);
    }
    GLuint vs_ref = s_quad_vertex_shader_ref;
    fprintf(stdout, "Compiling %s.\n", frag_shader_loc);
    GLuint fs_ref = get_shader(frag_shader_loc, GL_FRAGMENT_SHADER);
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
        fprintf(stderr, "%s\n%s\n", "Failed to link program:", buf);
    }
    glUseProgram(program);
    return program;
}

GLuint make_quad_program_from_string_source(const char *src) {
    if (s_quad_vertex_shader_ref < 0) {
        s_quad_vertex_shader_ref
            = make_vertex_shader(QUAD_VERTEX_SHADER_SOURCE);
    }
    GLuint vs_ref = s_quad_vertex_shader_ref;
    GLuint fs_ref = make_fragment_shader(src);
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

static GLuint to_base(int sized) {
    switch(sized) {
    case GL_RGBA32F: case GL_RGBA32I: case GL_RGBA32UI: case GL_RGBA16F:
    case GL_RGBA16I: case GL_RGBA16UI:
    case GL_RGBA8I: case GL_RGBA8UI: case GL_RGBA8:
        return GL_RGBA;
    case GL_RGB32F: case GL_RGB32I: case GL_RGB32UI: case GL_RGB16F:
    case GL_RGB16I: case GL_RGB16UI: case GL_RGB8I: case GL_RGB8UI:
    case GL_RGB8:
        return GL_RGB;
    case GL_RG32F: case GL_RG32I: case GL_RG32UI: case GL_RG16F:
    case GL_RG16I: case GL_RG16UI: case GL_RG8I: case GL_RG8UI:
        return GL_RG;
    case GL_R32F: case GL_R32I: case GL_R32UI: case GL_R16F:
    case GL_R16I: case GL_R16UI: case GL_R8: case GL_R8UI:
        return GL_RED;
    }
    return -1;
}

static GLuint to_type(int sized) {
    switch(sized) {
    case GL_RGBA32F: case GL_RGB32F: case GL_RG32F: case GL_R32F:
        return GL_FLOAT;
    case GL_RGBA32I: case GL_RGB32I: case GL_RG32I: case GL_R32I:
        return GL_INT;
    case GL_RGBA32UI: case GL_RGB32UI: case GL_RG32UI: case GL_R32UI:
        return GL_UNSIGNED_INT;
    case GL_RGBA16F: case GL_RGB16F: case GL_RG16F: case GL_R16F:
        return GL_HALF_FLOAT;
    case GL_RGBA16I: case GL_RGB16I: case GL_RG16I: case GL_R16I:
        return GL_SHORT;
    case GL_RGBA16UI: case GL_RGB16UI: case GL_RG16UI: case GL_R16UI:
        return GL_UNSIGNED_SHORT;
    case GL_RGBA8: case GL_RGB8: case GL_RG8: case GL_R8:
        return GL_BYTE;
    case GL_RGBA8UI: case GL_RGB8UI: case GL_RG8UI: case GL_R8UI:
        return GL_UNSIGNED_BYTE;
    }
    return -1;
}

void quad_init_texture(const struct TextureParams *params) {
    if (s_current_frame_id <= 0) {
        glActiveTexture(GL_TEXTURE0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        GL_LINEAR);
        /* {
            glActiveTexture(GL_TEXTURE0 + s_current_frame_id);
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            s_current_frame->texture = texture;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI,
                        512, 512, 0, 
                        to_base(GL_RGBA8UI),
                        to_type(GL_RGBA8UI),
                        NULL);
        }*/
        return;
    }
    glActiveTexture(GL_TEXTURE0 + s_current_frame_id);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    s_current_frame->texture = texture;
    glTexImage2D(GL_TEXTURE_2D, 0, params->format,
                 params->width, params->height, 0, 
                 to_base(params->format),
                 to_type(params->format),
                 NULL);
    if (params->generate_mipmap) glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params->wrap_s);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params->wrap_t);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    params->min_filter);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    params->mag_filter);
}


void quad_init_objects() {
    GLuint *vao_ptr = &s_current_frame->vao;
    GLuint *vbo_ptr = &s_current_frame->vbo;
    GLuint *ebo_ptr = &s_current_frame->ebo;
    GLuint *fbo_ptr = &s_current_frame->fbo;
    GLuint *texture_ptr = &s_current_frame->texture;
    glGenVertexArrays(1, vao_ptr);
    glBindVertexArray(*vao_ptr);
    glGenBuffers(1, vbo_ptr);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo_ptr);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES),
                    QUAD_VERTICES, GL_STATIC_DRAW);
    glGenBuffers(1, ebo_ptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo_ptr);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QUAD_ELEMENTS),
                 QUAD_ELEMENTS, GL_STATIC_DRAW);
    if (s_current_frame_id != 0) {
        glGenFramebuffers(1, fbo_ptr);
        glBindFramebuffer(GL_FRAMEBUFFER, *fbo_ptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                GL_TEXTURE_2D, *texture_ptr, 0);
    }
}

int pop_frame() {
    if (s_current_frame != NULL) {
        fprintf(stderr, ERR_FRAME_ACTIVE);
        return 0;
    }
    if (s_total_frames == 1) return s_total_frames - 1;
    s_current_frame = &s_frames[--s_total_frames];
    s_current_frame_id = s_total_frames;
    glDeleteTextures(1, &s_current_frame->texture);
    glDeleteRenderbuffers(1, &s_current_frame->rbo);
    glDeleteFramebuffers(1, &s_current_frame->fbo);
    glDeleteBuffers(1, &s_current_frame->ebo);
    glDeleteBuffers(1, &s_current_frame->vbo);
    glDeleteBuffers(1, &s_current_frame->vao);
    s_current_frame = NULL;
    return s_current_frame_id;
}

int new_frame(const struct TextureParams *texture_params,
              float *vertices, int sizeof_vertices,
              void *elements, int sizeof_elements) {
    if (s_current_frame != NULL) {
        fprintf(stderr, ERR_FRAME_ACTIVE);
        return -1;
    }
    int frame_id = s_total_frames++;
    if (s_total_frames > MAX_FRAME_COUNT) {
        fprintf(stderr, "Max number of frames %d exceeded.\n",
                --s_total_frames);
        return -1;
    }
    s_current_frame = &s_frames[frame_id];
    s_current_frame_id = frame_id;
    // init textures
    if (s_current_frame_id <= 0) {
        glActiveTexture(GL_TEXTURE0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        GL_LINEAR);}
    glActiveTexture(GL_TEXTURE0 + s_current_frame_id);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    s_current_frame->texture = texture;
    glTexImage2D(GL_TEXTURE_2D, 0, texture_params->format,
                 texture_params->width, texture_params->height, 0,
                 to_base(texture_params->format),
                 to_type(texture_params->format),
                 NULL);
    if (texture_params->generate_mipmap) glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    texture_params->wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    texture_params->wrap_t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    texture_params->min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    texture_params->mag_filter);

    // init objects
    GLuint *vao_ptr = &s_current_frame->vao;
    GLuint *vbo_ptr = &s_current_frame->vbo;
    GLuint *ebo_ptr = &s_current_frame->ebo;
    GLuint *fbo_ptr = &s_current_frame->fbo;
    GLuint *rbo_ptr = &s_current_frame->rbo;
    GLuint *texture_ptr = &s_current_frame->texture;
    glGenVertexArrays(1, vao_ptr);
    glBindVertexArray(*vao_ptr);
    glGenBuffers(1, vbo_ptr);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo_ptr);
    glBufferData(GL_ARRAY_BUFFER, sizeof_vertices,
                 vertices, GL_STATIC_DRAW);
    // if (elements != NULL) {
        glGenBuffers(1, ebo_ptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo_ptr);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_elements,
                     elements, GL_STATIC_DRAW);
        // }
    if (s_current_frame_id != 0) {
        glGenFramebuffers(1, fbo_ptr);
        glBindFramebuffer(GL_FRAMEBUFFER, *fbo_ptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, *texture_ptr, 0);
        glGenRenderbuffers(1, rbo_ptr);
        glBindRenderbuffer(GL_RENDERBUFFER, *rbo_ptr);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                              texture_params->width, texture_params->height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER, *rbo_ptr);

    }
    unbind();
    return frame_id;
}

int new_quad(const struct TextureParams *texture_params) {
    if (s_current_frame != NULL) {
        fprintf(stderr, ERR_FRAME_ACTIVE);
        return -1;
    }
    int quad_id = s_total_frames;
    s_total_frames += 1;
    if (s_total_frames > MAX_FRAME_COUNT) {
        fprintf(stderr, "Max number of frames %d exceeded.\n",
                --s_total_frames);
        return -1;
    }
    if (s_total_frames == 1 && texture_params != NULL) {
        fprintf(stdout, "Params ignored for first frame.\n");
    }
    s_current_frame = &s_frames[quad_id];
    s_current_frame_id = quad_id;
    quad_init_texture(texture_params);
    quad_init_objects();
    unbind();
    return quad_id;
}

void bind_frame(int frame2d_id, GLuint program) {
    if (s_current_frame != NULL) {
        fprintf(stderr, ERR_FRAME_ACTIVE);
        return;
    }
    if (frame2d_id > s_total_frames || frame2d_id < 0) {
        fprintf(stderr, "Invalid bind.\n");
        return;
    }
    s_current_frame_id = frame2d_id;
    s_current_frame = &s_frames[frame2d_id];
    s_current_frame->program = program;
    glUseProgram(program);
    glBindVertexArray(s_current_frame->vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_current_frame->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_current_frame->ebo);
    if (s_current_frame_id != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, s_current_frame->fbo);
        // glClear(GL_COLOR_BUFFER_BIT);
        glBindRenderbuffer(GL_RENDERBUFFER, s_current_frame->rbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

void change_program(GLuint program) {
    if (s_current_frame == NULL) {
        fprintf(stderr, ERR_NO_FRAME_ACTIVE);
        return;
    }
    s_current_frame->program = program;
    glUseProgram(program);
}

void set_vertex_attributes(const struct VertexParam *vertex_parameters,
                           int number_of_vertex_parameters) {
    for (int i = 0; i < number_of_vertex_parameters; i++) {
        GLint attrib = glGetAttribLocation(s_current_frame->program,
                                           vertex_parameters[i].name);
        glEnableVertexAttribArray(attrib);
        glVertexAttribPointer(attrib, // index
                              vertex_parameters[i].size,
                              vertex_parameters[i].type,
                              vertex_parameters[i].normalized,
                              vertex_parameters[i].stride,
                              (void *)vertex_parameters[i].offset);
    }

}

void bind_quad(int quad_id, GLuint program) {
    if (s_current_frame != NULL) {
        fprintf(stderr, ERR_FRAME_ACTIVE);
        return;
    }
    if (quad_id > s_total_frames || quad_id < 0) {
        fprintf(stderr, "Invalid bind.\n");
        return;
    }
    s_current_frame_id = quad_id;
    s_current_frame = &s_frames[quad_id];
    s_current_frame->program = program;
    glUseProgram(program);
    glBindVertexArray(s_current_frame->vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_current_frame->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_current_frame->ebo);
    if (s_current_frame_id != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, s_current_frame->fbo);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    GLint attrib = glGetAttribLocation(s_current_frame->program,
                                       "position");
    glEnableVertexAttribArray(attrib);
    glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, 3*4, 0);
}

void check_uniform_name_is_valid(const char *name, int ref) {
    if (ref < 0) {
        fprintf(stderr,
                "No such uniform \"%s\" for program %d.\n",
                name, s_current_frame->program);
    }
}

void set_int_uniform(const char *name, int val) {
    if (s_current_frame != NULL) {
        GLuint program = s_current_frame->program;
        GLuint loc = glGetUniformLocation(program, name);
        check_uniform_name_is_valid(name, loc);
        glUniform1i(loc, val);
        return;
    }
    if (s_err_msg_counter < 32) fprintf(stderr, ERR_NO_FRAME_ACTIVE);
    s_err_msg_counter++;
}

void set_sampler2D_uniform(const char *name, int val) {
    set_int_uniform(name, val);
}

void set_float_uniform(const char *name, float val) {
    if (s_current_frame != NULL) {
        GLuint program = s_current_frame->program;
        GLuint loc = glGetUniformLocation(program, name);
        check_uniform_name_is_valid(name, loc);
        glUniform1f(loc, val);
        return;
    }
    if (s_err_msg_counter < 32) fprintf(stderr, ERR_NO_FRAME_ACTIVE);
    s_err_msg_counter++;
}

void set_vec2_uniform(const char *name, float v0, float v1) {
    if (s_current_frame != NULL) {
        GLuint program = s_current_frame->program;
        GLuint loc = glGetUniformLocation(program, name);
        check_uniform_name_is_valid(name, loc);
        glUniform2f(loc, v0, v1);
        return;
    }
    if (s_err_msg_counter < 32) fprintf(stderr, ERR_NO_FRAME_ACTIVE);
    s_err_msg_counter++;
}

void set_vec3_uniform(const char *name, float v0, float v1, float v2) {
    if (s_current_frame != NULL) {
        GLuint program = s_current_frame->program;
        GLuint loc = glGetUniformLocation(program, name);
        check_uniform_name_is_valid(name, loc);
        glUniform3f(loc, v0, v1, v2);
        return;
    }
    if (s_err_msg_counter < 32) fprintf(stderr, ERR_NO_FRAME_ACTIVE);
    s_err_msg_counter++;
}

void set_vec4_uniform(const char *name,
                      float v0, float v1, float v2, float v3) {
    if (s_current_frame != NULL) {
        GLuint program = s_current_frame->program;
        GLuint loc = glGetUniformLocation(program, name);
        check_uniform_name_is_valid(name, loc);
        glUniform4f(loc, v0, v1, v2, v3);
        return;
    }
    if (s_err_msg_counter < 32) fprintf(stderr, ERR_NO_FRAME_ACTIVE);
    s_err_msg_counter++;
}

void set_ivec2_uniform(const char *name, int v0, int v1) {
    if (s_current_frame != NULL) {
        GLuint program = s_current_frame->program;
        GLuint loc = glGetUniformLocation(program, name);
        check_uniform_name_is_valid(name, loc);
        glUniform2i(loc, v0, v1);
        return;
    }
    if (s_err_msg_counter < 32) fprintf(stderr, ERR_NO_FRAME_ACTIVE);
    s_err_msg_counter++;
}

void set_ivec3_uniform(const char *name, int v0, int v1, int v2) {
    if (s_current_frame != NULL) {
        GLuint program = s_current_frame->program;
        GLuint loc = glGetUniformLocation(program, name);
        check_uniform_name_is_valid(name, loc);
        glUniform3i(loc, v0, v1, v2);
        return;
    }
    if (s_err_msg_counter < 32) fprintf(stderr, ERR_NO_FRAME_ACTIVE);
    s_err_msg_counter++;
}

void set_ivec4_uniform(const char *name, int v0, int v1, int v2, int v3) {
    if (s_current_frame != NULL) {
        GLuint program = s_current_frame->program;
        GLuint loc = glGetUniformLocation(program, name);
        check_uniform_name_is_valid(name, loc);
        glUniform4i(loc, v0, v1, v2, v3);
        return;
    }
    if (s_err_msg_counter < 32) fprintf(stderr, ERR_NO_FRAME_ACTIVE);
    s_err_msg_counter++;
}

void print_user_defined_uniforms() {
    if (s_current_frame == NULL) {
        fprintf(stderr, ERR_NO_FRAME_ACTIVE);
        return;
    }
    GLsizei buf_size = 512;
    GLsizei length = 0;
    GLint size_uniform = 0;
    GLenum type;
    char name[512] = {'\0',};
    for (int index = 0; glGetError() != GL_INVALID_VALUE;
         index++) {
        glGetActiveUniform(s_current_frame->program, index,
                           buf_size, &length, &size_uniform,
                           &type, name);
        char *type_name;
        const char MEDP_FLOAT[] = "mediump float";
        const char HIGHP_FLOAT[] = "highp float";
        const char STR_INT[] = "int";
        const char STR_UINT[] = "uint";
        float f_param;
        float f_params[4];
        int i_param;
        switch(type) {
        case GL_FLOAT: case GL_HALF_FLOAT:
            if (type==GL_HALF_FLOAT) type_name = (char *)MEDP_FLOAT;
            if (type==GL_FLOAT) type_name = (char *)HIGHP_FLOAT;
            glGetUniformfv(s_current_frame->program, index,
                           &f_param);
            fprintf(stdout, "%s %s = %f\n",
                    type_name, name, f_param);
            break;
        case GL_FLOAT_VEC2: case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
            glGetUniformfv(s_current_frame->program, index,
                           f_params);
            if (type == GL_FLOAT_VEC2) {
                fprintf(stdout, "vec2 %s = vec2(%f, %f)\n", name,
                        f_params[0], f_params[1]);
            } else if (type == GL_FLOAT_VEC3) {
                fprintf(stdout, "vec3 %s = vec3(%f, %f, %f)\n", name,
                        f_params[0], f_params[1], f_params[2]);
            } else {
                fprintf(stdout, "vec4 %s = vec4(%f, %f, %f, %f)\n", name,
                        f_params[0], f_params[1], f_params[2], f_params[3]);
            }
            break;
        case GL_INT: case GL_UNSIGNED_INT:
            // case GL_SHORT: case GL_UNSIGNED_SHORT:
            if (type==GL_INT) type_name = (char *)STR_INT;
            if (type==GL_UNSIGNED_INT) type_name =  (char *)STR_UINT;
            glGetUniformiv(s_current_frame->program, index,
                           &i_param);
            fprintf(stdout, "%s %s = %d\n",
                    type_name, name, i_param);
            break;
        case GL_SAMPLER_2D:
            glGetUniformiv(s_current_frame->program, index,
                           &i_param);
            fprintf(stdout, "sampler2D %s = %d\n",
                    name, i_param);
            break;
        default:
            fprintf(stdout, "%s of type %x\n", name, type);
        }
        /*if (glGetError() != GL_INVALID_VALUE) {
            fprintf(stdout, "%s, %d\n", name, type);
            }*/
    }
    puts("");
}

void draw_quad() {
    if (s_current_frame == NULL) {
        fprintf(stderr, ERR_NO_FRAME_ACTIVE);
        return;
    }
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void unbind() {
    if (s_current_frame != NULL) {
        s_current_frame_id = -1;
        s_current_frame = NULL;
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        return;
    }
    fprintf(stderr, ERR_NO_FRAME_ACTIVE);
}


void draw_unbind_quad() {
    draw_quad();
    unbind();
}

void get_rgb_unsigned_byte_array(int quad_id, int width, int height,
                                 unsigned char *array) {
    if (s_current_frame != NULL) {
        fprintf(stderr, ERR_FRAME_ACTIVE);
        return;
    }
    s_current_frame_id = quad_id;
    s_current_frame = &s_frames[quad_id];
    glBindVertexArray(s_current_frame->vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_current_frame->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_current_frame->ebo);
    if (s_current_frame_id != 0)
        glBindFramebuffer(GL_FRAMEBUFFER, s_current_frame->fbo);
    glActiveTexture(GL_TEXTURE0 + s_current_frame_id);
    glBindTexture(GL_TEXTURE_2D, s_current_frame->texture);
    glReadPixels(0, 0, width, height,
                 GL_RGB, GL_UNSIGNED_BYTE, array);
    unbind();
}

void get_quad_array(int quad_id, const struct TextureParams *tex_params,
                    void *array) {
    if (s_current_frame != NULL) {
        fprintf(stderr, ERR_FRAME_ACTIVE);
        return;
    }
    s_current_frame_id = quad_id;
    s_current_frame = &s_frames[quad_id];
    glBindVertexArray(s_current_frame->vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_current_frame->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_current_frame->ebo);
    if (s_current_frame_id != 0)
        glBindFramebuffer(GL_FRAMEBUFFER, s_current_frame->fbo);
    glActiveTexture(GL_TEXTURE0 + s_current_frame_id);
    glBindTexture(GL_TEXTURE_2D, s_current_frame->texture);
    glReadPixels(0, 0, tex_params->width, tex_params->height,
                 to_base(tex_params->format), to_type(tex_params->format),
                 array);
    unbind();
}

void get_quad_texture_array(int quad_id,
                            int x0, int y0, int width, int height,
                            int texture_type, void *array) {
    if (s_current_frame != NULL) {
        fprintf(stderr, ERR_FRAME_ACTIVE);
        return;
    }
    s_current_frame_id = quad_id;
    s_current_frame = &s_frames[quad_id];
    glBindVertexArray(s_current_frame->vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_current_frame->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_current_frame->ebo);
    if (s_current_frame_id != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, s_current_frame->fbo);
    };
    glActiveTexture(GL_TEXTURE0 + s_current_frame_id);
    glBindTexture(GL_TEXTURE_2D, s_current_frame->texture);
    if (texture_type == GL_UNSIGNED_BYTE) {
        glReadPixels(x0, y0, width, height, GL_RGBA,
                     GL_UNSIGNED_BYTE, array);
    } else if (texture_type == GL_FLOAT) {
        glReadPixels(x0, y0, width, height,
                     GL_RGBA, GL_FLOAT, array);
    } else {
        fprintf(stderr, "Not supported.\n");
        unbind();
        return;
    }
    unbind();
}

void quad_substitute_array(int quad_id, const struct TextureParams *tex_params,
                           void *array) {
    if (s_current_frame != NULL) {
        fprintf(stderr, ERR_FRAME_ACTIVE);
        return;
    }
    s_current_frame_id = quad_id;
    s_current_frame = &s_frames[quad_id];
    glBindVertexArray(s_current_frame->vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_current_frame->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_current_frame->ebo);
    if (s_current_frame_id != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, s_current_frame->fbo);
    };
    glActiveTexture(GL_TEXTURE0 + s_current_frame_id);
    glBindTexture(GL_TEXTURE_2D, s_current_frame->texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                    tex_params->width, tex_params->height,
                    to_base(tex_params->format), 
                    to_type(tex_params->format), array);
    // glTexImage2D(GL_TEXTURE_2D, 0, 4,
    //              width, height, 0, GL_RGBA, texture_type,
    //              array);
    // glActiveTexture(GL_TEXTURE0);
    unbind();
}

void window_dimensions(GLFWwindow *window, int *ptr_w, int *ptr_h) {
    glfwGetWindowSize(window, ptr_w, ptr_h);
#ifdef __APPLE__
    *ptr_w = 2**ptr_w;
    *ptr_h = 2**ptr_h;
#endif
}
