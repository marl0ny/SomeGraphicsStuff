#include "gl_wrappers.h"

namespace gl_wrappers {

    void CubeMapFrame::init_texture(int width, int height,
                                    int texture_type) {
        glActiveTexture(GL_TEXTURE0 + frame_number);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        for (size_t i = 0; i < 6; i++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,
                         0, GL_RGBA, width, height, 0, GL_RGB,
                         texture_type, nullptr);
        }
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        glTextureParameteri(GL_TEXTURE_CUBE_MAP,
                            GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(GL_TEXTURE_CUBE_MAP,
                            GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(GL_TEXTURE_CUBE_MAP,
                            GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(GL_TEXTURE_CUBE_MAP,
                            GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(GL_TEXTURE_CUBE_MAP,
                            GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }

    CubeMapFrame::CubeMapFrame(int width, int height) {
        CubeMapFrame::total_frames += 1;
        this->frame_number = CubeMapFrame::total_frames - 1;
        init_texture(width, height, GL_UNSIGNED_BYTE);
        float vertices[] = {
            1.0, 1.0, -1.0,
            1.0, -1.0, -1.0,
            -1.0, 1.0, -1.0,
            -1.0, -1.0, -1.0,
            1.0, -1.0, 1.0,
            1.0, 1.0, 1.0,
            -1.0, -1.0, 1.0,
            -1.0, 1.0, 1.0
        };
        float elements[] = {
            0, 1, 2,
            2, 3, 1,
            1, 4, 0,
            0, 5, 4,
            4, 5, 6,
            6, 5, 7,
            7, 6, 3,
            3, 2, 7,
            0, 2, 7,
            7, 5, 0,
            1, 3, 4,
            4, 6, 3
        };
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                        vertices, GL_STATIC_DRAW);
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements,
                     GL_STATIC_DRAW);
        if (frame_number != 0) {
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                    GL_TEXTURE_CUBE_MAP, texture, 0);
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                                  width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                      GL_DEPTH_ATTACHMENT,
                                      GL_RENDERBUFFER, rbo
                                      );
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
            GL_FRAMEBUFFER_COMPLETE) {
                std::cout << "framebuffer incomplete" << std::endl;
        }
    }

    void CubeMapFrame::bind() {
        glBindVertexArray(vao);
        if (frame_number != 0) {
            glBindFramebuffer(GL_RENDERBUFFER, rbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        GLuint attrib = glGetAttribLocation(program, "position");
        glEnableVertexAttribArray(attrib);
        glVertexAttribPointer(attrib, 3, GL_FLOAT, false, 3*4, 0);
    }

    void CubeMapFrame::bind(GLuint program) {
        set_program(program);
        bind();
    }

    void CubeMapFrame::draw() {
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

}
