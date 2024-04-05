#include "gl_wrappers/gl_wrappers.h"
#include "texture_data.hpp"
#include "quaternions.hpp"

#ifndef _VOL_RENDER_
#define _VOL_RENDER_

/*
Work In Progress Volumetric Rendering.

References
 - [Wikipedia - Volume rendering]
 (https://en.wikipedia.org/wiki/Volume_rendering)
 - [Wikipedia - Volume ray casting]
 (https://en.wikipedia.org/wiki/Volume_ray_casting)
 - [Learn OpenGL - Blending]
 (https://learnopengl.com/Advanced-OpenGL/Blending)

*/
class VolumeRender {

    // This is used to rotate the vertices of the volume render frames.
    Quaternion debug_rotation;

    // The dimensions of the final output texture
    // that is displayed on screen.
    IVec2 view_dimensions;

    // 2D texture dimensions
    // 2D dimensions of the inital volume data texture
    IVec2 sample_texel_dimensions_2d;
    // 2D dimensions of the texture used in the volume render frame
    IVec2 render_texel_dimensions_2d;

    // 3D texture dimensions
    // 3D dimensions of the inital volume data texture
    IVec3 sample_texel_dimensions_3d;
    // 3D dimensions of the texture used in the volume render frame
    IVec3 render_texel_dimensions_3d;
    
    // Vec3 translate;
    struct { // Perhaps this should be static?
        GLuint gradient, sample_volume, show_volume;
    } programs;
    struct {
        // frame_id draw;
        frame_id gradient;
        // frame_id boundary_mask;
        frame_id vol_half_precision;
        frame_id gradient_half_precision;
        frame_id sample_volume;
        frame_id sample_grad;
        frame_id out;
        // frame_id sub_view1, sub_view2;
        // frame_id sub_view3, sub_view4;
    } frames;
    int sizeof_elements;
    enum {
        Z_ORIENTATION=0, X_ORIENTATION=1, Y_ORIENTATION=2
    };
    void init_programs();
    void init_frames();
    public:
    VolumeRender(IVec2 view_dimensions,
                 IVec3 render_dimensions, IVec3 sample_dimensions);
    Texture2DData render(const Texture2DData &volume_data,
                         float view_scale, Quaternion rotation
                         ) const;
    Texture2DData debug_get_vol_half_precision() const;
    Texture2DData debug_get_grad_half_precision() const;
    Texture2DData debug_get_sample_grad() const;
    Texture2DData debug_get_sample_vol() const;

};

#endif

