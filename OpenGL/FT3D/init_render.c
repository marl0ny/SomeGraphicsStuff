#include <GLES3/gl3.h>
#include <math.h>
#include "init_render.h"
#include <stdio.h>
#include <stdlib.h>
#include "gl_wrappers/gl_wrappers.h"
#include "fft_gl.h"
#include "texture_dimensions.h"


struct Programs {
    GLuint copy, zero, scale;
    GLuint points_density;
    GLuint init_gaussian;
    GLuint init_boundary;
    GLuint fft_iter;
    GLuint rev_bit_sort2;
    GLuint fftshift;
    GLuint view;
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
    frame_id fftshift[2];
    frame_id draw;
    frame_id draw2;
    frame_id fft_iters[2];
};

static struct Programs s_programs = {};
static struct SimParams s_sim_params = {};
static struct Frames s_frames = {};


void init_programs(struct Programs *programs) {
    programs->zero = make_quad_program("./shaders/zero.frag");
    programs->copy = make_quad_program("./shaders/copy.frag");
    programs->scale = make_quad_program("./shaders/scale.frag");
    programs->init_gaussian
        = make_quad_program("./shaders/init-gaussian.frag");
    programs->fft_iter = make_quad_program("./shaders/fft-iter.frag");
    programs->rev_bit_sort2
        = make_quad_program("./shaders/rev-bit-sort2.frag");
    programs->fftshift = make_quad_program("./shaders/fftshift.frag");
    programs->view = make_quad_program("./shaders/view.frag");
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
    frames->draw2 = new_quad(&tex_params);
    for (int i = 0; i < 3; i++)
        frames->fftshift[i] = new_quad(&tex_params);
    for (int i = 0; i < 2; i++)
        frames->fft_iters[i] = new_quad(&tex_params);
    
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
    set_float_uniform("amplitude", (float)render_params->inc_mode2/100.0);
    set_vec3_uniform("r0",
                     (float)render_params->move_x/100.0,
                     (float)render_params->move_y/100.0,
                     (float)render_params->move_z/100.0);
    float nx = (render_params->x - 0.5)*tex_dimensions->width_3d;
    float ny = (render_params->y - 0.5)*tex_dimensions->height_3d;
    set_vec3_uniform("direction", nx, ny, (float)render_params->inc_mode1);
    set_vec3_uniform("sigma",
                     render_params->scroll,
                     render_params->scroll,
                     render_params->scroll);
    draw_unbind_quad();

    int res = fft3d(s_programs.rev_bit_sort2, s_programs.fft_iter, 
                    s_frames.draw, s_frames.fft_iters[0], s_frames.fft_iters[1],
                    tex_dimensions);
    int fftshift_res = fftshift3d(s_programs.fftshift, res,
                                  s_frames.fftshift[0], s_frames.fftshift[1],
                                  tex_dimensions);
    
    bind_quad(s_frames.draw2, s_programs.copy);
    set_sampler2D_uniform("tex", res);
    draw_unbind_quad();

    res = ifft3d(s_programs.rev_bit_sort2, s_programs.fft_iter, 
                 s_frames.draw2, s_frames.fft_iters[0], s_frames.fft_iters[1],
                 tex_dimensions);

    #ifdef __APPLE__
    glViewport(0, 0, 2*s_sim_params.tex_dimensions.width_2d,
               2*s_sim_params.tex_dimensions.height_2d);
    #endif
    bind_quad(s_frames.main_view, s_programs.view);
    switch(render_params->view_mode) {
        case 0:
        set_sampler2D_uniform("tex", s_frames.draw);
        break;
        case 1:
        set_sampler2D_uniform("tex", fftshift_res);
        break;
        case 2:
        set_sampler2D_uniform("tex", res);
        break;
    }
    draw_unbind_quad();
}
