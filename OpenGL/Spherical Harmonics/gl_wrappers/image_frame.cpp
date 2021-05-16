#include "gl_wrappers.h"

namespace gl_wrappers {

    ImageFrame::ImageFrame(int w, int h,
                           uint8_t *image,
                           GLuint channel_type) {
        ImageFrame::total_frames += 1;
        glActiveTexture(GL_TEXTURE0 + ImageFrame::frame_number);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        if (channel_type == GL_RGB) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
                        w, h, 0, channel_type,
                        GL_UNSIGNED_BYTE, image);
        } else if (channel_type == GL_RGBA) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                        w, h, 0, channel_type,
                        GL_UNSIGNED_BYTE, image);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void ImageFrame::draw() {
        glActiveTexture(GL_TEXTURE0 + this->frame_number);
        glBindTexture(GL_TEXTURE_2D, this->texture);
    };
}
