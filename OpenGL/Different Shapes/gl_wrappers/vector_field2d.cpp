#include "gl_wrappers.h"


namespace gl_wrappers {

    VectorField2D::VectorField2D(std::vector<float> &vert_data,
                                 int width, int height, int texture_type) {
        VectorField2D::total_frames += 1;
        this->frame_number = VectorField2D::total_frames - 1;
        this->init_texture(width, height, texture_type);
        init_objects(&vert_data[0], vert_data.size());
    }

    void VectorField2D::init_texture(int width, int height,
                                     int texture_type) {
        this->width = width;
        this->height = height;
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

    void VectorField2D::init_objects(float *vert_data,
                                     int vertex_data_size) {
        int n = vertex_data_size;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, n*sizeof(float),
                        vert_data, GL_STATIC_DRAW);
        if (frame_number != 0) {
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            // glGenRenderbuffers(1, &rbo);
            // glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
            //                       width, height);
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                                  width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                      GL_DEPTH_ATTACHMENT,
                                      GL_RENDERBUFFER, rbo
                                      );
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, texture, 0);
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
            GL_FRAMEBUFFER_COMPLETE) {
                std::cout << "framebuffer incomplete" << std::endl;
            }
        this->vertex_data_size = n;
    }

    void VectorField2D::bind() {
        if (frame_number != 0) {
            glBindFramebuffer(GL_RENDERBUFFER, rbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        }
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }

    void VectorField2D::set_attribute(const std::string &attrib_name, int size,
                                      int vertex_size, size_t offset,
                                      int type, int normalize) {
        GLuint attrib = glGetAttribLocation(program, attrib_name.c_str());
        glEnableVertexAttribArray(attrib);
        size_t offset_bytes = sizeof(float)*offset;
        glVertexAttribPointer(attrib, size, type, normalize,
                              vertex_size*sizeof(float),
                              (const void *)offset_bytes);
    }

    void VectorField2D::set_attributes(const std::vector
                                       <std::pair<std::string, int> >
                                       &attribute) {
        auto bin_op = [](int val, std::pair<std::string, int> pair) {
            return val + pair.second;
        };
        int total_size = std::accumulate(attribute.begin(), attribute.end(),
                                         0, bin_op);
        int offset = 0;
        for (auto &a: attribute) {
            // std::cout << a.first << offset << std::endl;
            set_attribute(a.first, a.second, total_size, offset);
            offset += a.second;
        }
        this->vertices_index_count = vertex_data_size/total_size;
    }

    void VectorField2D::bind(GLuint program) {
        this->set_program(program);
        this->bind();
    }

    void VectorField2D::bind_array(std::vector<float> &arr) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, arr.size()*sizeof(float),
                        &arr[0], GL_STATIC_DRAW);
        this->vertex_data_size = arr.size();
    }

    void VectorField2D::draw() {
        glDrawArrays(GL_LINES, 0, this->vertices_index_count);
    }

}
