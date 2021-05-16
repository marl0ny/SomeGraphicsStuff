#include "gl_wrappers.h"

namespace gl_wrappers {

    Quad::Quad(int width, int height, int type, 
               const std::string &vertex_name) {
        Quad::total_frames += 1;
        frame_number = Quad::total_frames - 1;
        for (int i = 0; i < 15; i++) {
            this->vertex_name[i] = vertex_name[i];
        }
        init_texture(width, height, type);
        init_objects();
        unbind();
    }

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
        // set_vertex_name(vertex_name);
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

    void Quad::bind(GLuint program) {
        Quad::set_program(program);
        Quad::bind();
    }

    void Quad::set_vertex_name(const std::string &name) {
        //std::cout << name << std::endl;
        //std::cout << program << std::endl;
        GLint attrib = glGetAttribLocation(program, name.c_str());
        glEnableVertexAttribArray(attrib);
        glVertexAttribPointer(attrib, 3, GL_FLOAT, false, 3*4, 0);
    }

    void Quad::set_vertex_name(const char *name) {
        //std::cout << name << std::endl;
        //std::cout << program << std::endl;
        GLint attrib = glGetAttribLocation(program, name);
        glEnableVertexAttribArray(attrib);
        glVertexAttribPointer(attrib, 3, GL_FLOAT, false, 3*4, 0);
    }

    int Quad::get_texture() const {
        return texture;
    }

    void Quad::draw() const {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

}
