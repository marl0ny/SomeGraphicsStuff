#include "gl_wrappers.hpp"

#ifndef _ARROWS3D_
#define _ARROWS3D_

namespace arrows3d {

WireFrame get_3d_vector_field_wire_frame(IVec3 d_3d);

struct Programs {
    unsigned int arrows3d;
    // unsigned int 
    Programs();
};

struct Frames {
    WireFrame arrows3d;
    Frames(const TextureParams &default_texture_params,
           IVec3 d_3d);
};


class Arrows {
    Programs m_programs;
    Frames m_frames;
    IVec3 m_dimensions_3d;
    public:
    Arrows(IVec3 d_3d, TextureParams default_tex_params);
    void view(
        RenderTarget &dst, const Quad &src,
        float scale, Quaternion rotation,
        Uniforms additional_uniforms = {});
};

}

#endif