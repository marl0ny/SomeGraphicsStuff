#include "interface.h"
#include <GLES3/gl3.h>
#include <math.h>
#include "interface.h"
#include <stdio.h>
#include <stdlib.h>
#include "gl_wrappers/gl_wrappers.h"


struct Programs {
    GLuint copy, zero, scale, colour;
    GLuint points_density;
    GLuint init_gaussian;
    GLuint init_boundary;
    GLuint fft_iter;
    GLuint rev_bit_sort2;
};

struct TextureDimensions {
    int width_3d, height_3d, length_3d;
    int width_2d, height_2d;
};

struct SimParams {
    int view_width, view_height;
    int sizeof_vertices;
    int sizeof_elements;
    struct TextureDimensions tex_dimensions;
    struct DVec4 rotation;
    float scale;
    struct Vec3 translate;
};

struct Frames {
    frame_id main_view;
    frame_id sub_views[3];
    frame_id draw;
    frame_id fft_iters[10];
};

static struct Programs s_programs = {};
static struct SimParams s_sim_params = {};
static struct Frames s_frames = {};

void init_programs(struct Programs *programs) {
    programs->zero = make_quad_program("./shaders/zero.frag");
    programs->copy = make_quad_program("./shaders/copy.frag");
    programs->scale = make_quad_program("./shaders/scale.frag");
    programs->colour = make_quad_program("./shaders/colour.frag");
    programs->init_gaussian
        = make_quad_program("./shaders/init-gaussian.frag");
    programs->fft_iter = make_quad_program("./shaders/fft-iter.frag");
    programs->rev_bit_sort2
        = make_quad_program("./shaders/rev-bit-sort2.frag");
}

void init_sim_params(struct SimParams *params) {
    #ifdef __APPLE__
    params->view_width = 1024;
    params->view_height = 1024;
    #else
    params->view_width = 512;
    params->view_height = 512;
    #endif
    params->rotation.x = 0.0;
    params->rotation.y = 0.0;
    params->rotation.z = 0.0;
    params->rotation.w = 1.0;
    params->scale = 1.0;
    params->translate.x = -0.5;
    params->translate.y = -0.5;
    params->translate.z = -0.5;
    params->tex_dimensions.width_2d = 512;
    params->tex_dimensions.height_2d = 512;
    params->tex_dimensions.width_3d = 64;
    params->tex_dimensions.height_3d = 64;
    params->tex_dimensions.length_3d = 64;
    params->sizeof_vertices = (4*sizeof(float)*
                               params->tex_dimensions.width_2d*
                               params->tex_dimensions.height_2d);
}

void init_frames(struct Frames *frames, const struct SimParams *params) {
    frames->main_view = new_quad(NULL);
    struct TextureParams tex_params = {
        .type=GL_FLOAT,
        .width=params->tex_dimensions.width_2d,
        .height=params->tex_dimensions.height_2d,
        .generate_mipmap=1, .wrap_s=GL_CLAMP_TO_EDGE, .wrap_t=GL_CLAMP_TO_EDGE,
        .min_filter=GL_LINEAR, .mag_filter=GL_LINEAR
    };
    frames->draw = new_quad(&tex_params);
    for (int i = 0; i < 3; i++)
        frames->sub_views[i] = new_quad(&tex_params);
    for (int i = 0; i < 10; i++)
        frames->fft_iters[i] = new_quad(&tex_params);
    
}

void swap2(frame_id *f1, frame_id *f2) {
    frame_id tmp = *f2;
    *f2 = *f1;
    *f1 = tmp;
}

const int ORIENTATION_0 = 0;
const int ORIENTATION_1 = 1;
const int ORIENTATION_2 = 2;
frame_id fft_iter(int fft_iter_program, 
                  frame_id fft_iter1, frame_id fft_iter2,
                  int orientation,
                  const struct TextureDimensions *tex_dimensions) {
    frame_id iter[2] = {fft_iter1, fft_iter2};
    int size = 0;
    switch(orientation) {
        case ORIENTATION_0:
        size = tex_dimensions->width_3d;
        break;
        case ORIENTATION_1:
        size = tex_dimensions->height_3d;
        break;
        case ORIENTATION_2:
        size = tex_dimensions->length_3d;
        break;
    }
    for (float block_size = 2.0; block_size <= size; block_size *= 2.0) {
        bind_quad(iter[1], fft_iter_program);
        set_ivec2_uniform("texelDimensions2D",
                          tex_dimensions->width_2d, tex_dimensions->height_2d);
        set_ivec3_uniform("texelDimensions3D",
                          tex_dimensions->width_3d, tex_dimensions->height_3d, 
                          tex_dimensions->length_3d);
        set_sampler2D_uniform("tex", iter[0]);
        set_int_uniform("orientation", orientation);
        set_float_uniform("blockSize", block_size/(float)size);
        set_float_uniform("size", (float)size);
        set_float_uniform("angleSign", -1.0);
        set_float_uniform("scale", 1.0);
        draw_unbind_quad();
        swap2(&iter[0], &iter[1]);
    }
    return iter[0];
}

void init() {
    init_programs(&s_programs);
    init_sim_params(&s_sim_params);
    init_frames(&s_frames, &s_sim_params);
    glViewport(0, 0, s_sim_params.tex_dimensions.width_2d,
               s_sim_params.tex_dimensions.height_2d);
    // 
}

void render(const struct RenderParams *render_params) {
    glViewport(0, 0, s_sim_params.tex_dimensions.width_2d,
               s_sim_params.tex_dimensions.height_2d);
    struct TextureDimensions *tex_dimensions = &s_sim_params.tex_dimensions;
    bind_quad(s_frames.draw, s_programs.init_gaussian);
    set_ivec2_uniform("texelDimensions2D",
                      tex_dimensions->width_2d, tex_dimensions->height_2d);
    set_ivec3_uniform("texelDimensions3D",
                      tex_dimensions->width_3d, tex_dimensions->height_3d, 
                      tex_dimensions->length_3d);
    set_vec3_uniform("r0", 0.5, 0.5, 0.5);
    set_vec3_uniform("colour", 100.0, 0.0, 0.0);
    set_vec3_uniform("sigma", 0.25, 0.25, 0.25);
    draw_unbind_quad();
    bind_quad(s_frames.fft_iters[0], s_programs.rev_bit_sort2);
    set_ivec2_uniform("texelDimensions2D",
                      tex_dimensions->width_2d, tex_dimensions->height_2d);
    set_ivec3_uniform("texelDimensions3D",
                      tex_dimensions->width_3d, tex_dimensions->height_3d, 
                      tex_dimensions->length_3d);
    set_vec3_uniform("dr", 1.0, 1.0, 1.0);
    set_vec3_uniform("dimensions3D", (float)tex_dimensions->width_3d,
                     (float)tex_dimensions->height_3d,
                     (float)tex_dimensions->length_3d);               
    set_sampler2D_uniform("tex", s_frames.draw);
    draw_unbind_quad();
    int res_x = fft_iter(s_programs.fft_iter, 
                        s_frames.fft_iters[0], s_frames.fft_iters[1],
                        ORIENTATION_0, &s_sim_params.tex_dimensions);
    if (res_x != s_frames.fft_iters[0])
        swap2(&s_frames.fft_iters[0], &s_frames.fft_iters[1]);
    int res_y = fft_iter(s_programs.fft_iter, 
                        s_frames.fft_iters[0], s_frames.fft_iters[1],
                        ORIENTATION_1, &s_sim_params.tex_dimensions);
    if (res_y != s_frames.fft_iters[0])
        swap2(&s_frames.fft_iters[0], &s_frames.fft_iters[1]);
    int res_z = fft_iter(s_programs.fft_iter, 
                        s_frames.fft_iters[0], s_frames.fft_iters[1],
                        ORIENTATION_2, &s_sim_params.tex_dimensions);
    #ifdef __APPLE__
    glViewport(0, 0, 2*s_sim_params.tex_dimensions.width_2d,
               2*s_sim_params.tex_dimensions.height_2d);
    #endif
    bind_quad(s_frames.main_view, s_programs.copy);
    set_sampler2D_uniform("tex", res_z);
    draw_unbind_quad();
}
