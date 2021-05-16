#include "gl_wrappers.h"

namespace gl_wrappers {

    int Frame::total_frames = 0;

    int Frame::get_value() const {
        return frame_number;
    }

    int Frame::get_tex_unit() const {
        return GL_TEXTURE0 + get_value();
    }
}
