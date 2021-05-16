#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <map>
#include <numeric>
#include <utility>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdint>
#ifndef _GL_WRAPPERS_HPP_
#define _GL_WRAPPERS_HPP_


namespace gl_wrappers {

    GLFWwindow *init_window(int width, int height);
    void init_glew();
    GLuint make_texture(uint8_t *image, size_t image_w, size_t image_h);
    GLuint make_float_texture(float *image, size_t image_w, size_t image_h);
    void unbind();
    GLuint make_program(GLuint vs_ref, GLuint fs_ref);
    GLuint make_vertex_shader(const char *v_source);
    GLuint make_fragment_shader(const char *f_source);
    GLuint get_shader(const char *shader_loc, GLuint shader_type);

    class Frame {
        Frame(Frame &);
        Frame(Frame &&);
        Frame &operator=(Frame &);
        Frame &operator=(Frame &&);
        protected:
        GLuint program = 0;
        static int total_frames;
        int frame_number = 0;
        Frame() {};
        public:
        int get_value() const;
        int get_tex_unit() const;
        static int get_blank() {
            return total_frames;
        }
    };

    class ImageFrame: public Frame {
        GLuint texture = 0;
        public:
        ImageFrame(int w, int h, uint8_t *image,
                   GLuint channel_type=GL_RGB);
        void draw();
        ~ImageFrame() {
            glDeleteTextures(1, &texture);
        }
    };

    class ProgrammableFrame: public Frame {
        protected:
        GLuint program = 0;
        ProgrammableFrame() {};
        public:
        void set_program(GLuint program);
        void set_int_uniform(const char *name, int val);
        void set_int_uniform(const std::string &name, int val);
        void set_int_uniforms(const std::map<std::string, int> &uniforms);
        void set_float_uniform(const char *name, float val);
        void set_float_uniform(const std::string &name, float val);
        void set_float_uniforms(const std::map<std::string, double> &uniforms);
        void set_vec2_uniform(const char *name, const float *v);
        void set_vec2_uniform(const char *name, const std::vector<float> &v);
        void set_vec2_uniform(const char *name, float v0, float v1);
        void set_vec2_uniforms(const std::map<std::string,
                                              std::pair<float, float>
                                             > &uniforms);
        void set_vec3_uniform(const char *name, const float *v);
        void set_vec3_uniform(const char *name, const std::vector<float> &v);
        void set_vec3_uniform(const char *name,
                              float v0, float v1, float v2);
        void set_vec4_uniform(const char *name, const float *v);
        void set_vec4_uniform(const char *name, const std::vector<float> &v);
        void set_vec4_uniform(const char *name,
                              float v0, float v1, float v2, float v3);
    };

    class Frame3D: public ProgrammableFrame {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;
        GLuint fbo = 0;
        GLuint rbo = 0;
        GLuint texture = 0;
        bool is_edge = false;
        int vertex_data_size = 0;
        int edge_data_size = 0;
        int vertices_index_count = 0;
        int width = 0;
        int height = 0;
        void init_objects(float *vertex_data, int *edge_data,
                        int vertex_data_size, int edge_data_size);
        void init_objects(float *vertex_data, int vertex_data_size);
        void init_texture(int width, int height, int texture_type);
        public:
        Frame3D(std::vector<float> &vert_data);
        Frame3D(std::vector<float> &vertex_data, std::vector<int> &edge_data);
        Frame3D(std::vector<float> &vertex_data, std::vector<int> &edge_data,
                int width, int height, int texture_type);
        Frame3D(std::vector<float> &vertex_data, int width, int height,
                int texture_type);
        void bind_array(float *vertex_data, int vertex_data_size);
        void bind_array(std::vector<float> &vertex_data);
        void bind_arrays(float *vertex_data, int *edge_data,
                        int vertex_data_size, int edge_data_size);
        void bind_arrays(std::vector<float> &vertex_data,
                        std::vector<int> &edge_data);
        void set_number_of_vertices(int n);
        void bind();
        void bind(GLuint program);
        void get_texture() const;
        void set_attribute(const std::string &attrib_name, int size,
                            int vertex_size, size_t offset,
                            int type=GL_FLOAT, int normalize=GL_FALSE);
        void set_attributes(const std::vector<std::pair<std::string, int>>
                            &attribute);
        void draw() const;
        ~Frame3D() {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &ebo);
            if (frame_number != 0) {
                glDeleteFramebuffers(1, &fbo);
                glDeleteRenderbuffers(1, &rbo);
            }
            glDeleteTextures(1, &texture);
        }
    };

    class CubeMapFrame: public ProgrammableFrame {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;
        GLuint fbo = 0;
        GLuint rbo = 0;
        GLuint texture = 0;
        void init_texture(int width, int height, int texture_type);
        public:
        CubeMapFrame(int width, int height);
        CubeMapFrame(int width, int height, uint8_t *texture);
        void bind();
        void bind(GLuint program);
        int get_texture() const;
        void draw();

    };

    class Quad: public ProgrammableFrame {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;
        GLuint fbo = 0;
        GLuint texture = 0;
        char vertex_name[16] = {0};
        void init_texture(int width, int height, int texture_type);
        void init_objects();
        public:
        void set_vertex_name(const char *name);
        void set_vertex_name(const std::string &name);
        void bind();
        void bind(GLuint program);
        int get_texture() const;
        Quad(int width, int height, int type=GL_FLOAT,
             const std::string &vertex_name=
             std::string("position"));
        void draw() const;
        ~Quad() {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &ebo);
            if (frame_number != 0) {
                glDeleteBuffers(1, &fbo);
            }
            glDeleteTextures(1, &texture);
        }
    };

}

#endif
