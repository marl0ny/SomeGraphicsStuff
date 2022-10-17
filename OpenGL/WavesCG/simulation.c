#include "simulation.h"
#include "summation_gl.h"
#include <GLES3/gl3.h>
#include <math.h>
#include "cg_gl.h"

static const float PI = 3.141592653589793;

void init_sim_params(struct SimParams *params) {
    int width = START_WIDTH;
    int height = START_HEIGHT;
    params->texel_width = width;
    params->texel_height = height;
    params->width = (float)width;
    params->height = (float)height;
    params->dt = 0.5;
    params->dx = params->width/params->texel_width;
    params->dy = params->height/params->texel_height;
    params->init_wave.amplitude.x = 1.0;
    params->init_wave.amplitude.y = 1.0;
    params->init_wave.amplitude.z = 1.0;
    params->init_wave.amplitude.w = 1.0;
    params->init_wave.u0 = 0.5;
    params->init_wave.v0 = 0.5;
    params->init_wave.sigma_x = 0.1;
    params->init_wave.sigma_y = 0.1;
}

void init_programs(struct Programs *programs) {
    programs->zero = make_program("./shaders/zero.frag");
    programs->copy = make_program("./shaders/copy.frag");
    programs->resize_copy = make_program("./shaders/resize-copy.frag");
    programs->copy2 = make_program("./shaders/copy2.frag");
    programs->scale = make_program("./shaders/scale.frag");
    programs->add2 = make_program("./shaders/add2.frag");
    programs->multiply = make_program("./shaders/multiply.frag");
    programs->init_dist = make_program("./shaders/init-dist.frag");
    programs->explicit_part = make_program("./shaders/explicit-part.frag");
    programs->implicit_part = make_program("./shaders/implicit-part.frag");
    programs->view = make_program("./shaders/view.frag");
}

void init_frames(struct Frames *quads, const struct SimParams *params) {
    int width = params->texel_width, height = params->texel_height;
    struct TextureParams texture_params = {
        .type=GL_FLOAT, .width=width, .height=height,
        .generate_mipmap=1,
        .wrap_s=GL_CLAMP_TO_EDGE, .wrap_t=GL_CLAMP_TO_EDGE,
        .min_filter=GL_LINEAR, .mag_filter=GL_LINEAR,
    };
    quads->main_view = new_quad(NULL);
    quads->secondary_view = new_quad(&texture_params);
    for (int i = 0; i < N_SIM_FRAMES; i++) {
        quads->simulation[i] = new_quad(&texture_params);
    }
    for (int i = 0; i < N_ITER_FRAMES; i++) {
        quads->iter_solver[i] = new_quad(&texture_params);
    }

    texture_params.min_filter = GL_LINEAR;
    texture_params.mag_filter = GL_LINEAR;
    for (int i = 0; i < SUM_FRAMES; i++) {
        texture_params.width /= 2;
        texture_params.height /= 2;
        quads->summations[i] = new_quad(&texture_params);
    }
}

void set_dx_dy_width_height(float dx, float dy, float width, float height) {
    set_float_uniform("dx", dx);
    set_float_uniform("dy", dy);
    set_float_uniform("width", width);
    set_float_uniform("height", height);
}

static void roll3(frame_id *f1, frame_id *f2, frame_id *f3) {
    frame_id f1_tmp  = *f1, f2_tmp = *f2, f3_tmp = *f3;
    *f1 = f2_tmp, *f2 = f3_tmp , *f3 = f1_tmp;
}

struct Vec4 dot(frame_id multiply_program, 
                frame_id scale_program,
                frame_id *sum_quads, int size,
                frame_id v1, frame_id v2, frame_id v1_v2) {
    bind_quad(v1_v2, multiply_program);
    set_sampler2D_uniform("tex1", v1);
    set_sampler2D_uniform("tex2", v2);
    draw_unbind();
    return texture_reduction_sum(scale_program,
                                 v1_v2, sum_quads,
                                 size);
}

void add(GLuint add_program,
         const struct Vec4 *s1, frame_id v1,
         const struct Vec4 *s2, frame_id v2,
         frame_id res) {
    bind_quad(res, add_program);
    set_vec4_uniform("scale1",
                     s1->ind[0], s1->ind[1],
                     s1->ind[2], s1->ind[3]);
    set_sampler2D_uniform("tex1", v1);
    set_vec4_uniform("scale2",
                     s2->ind[0], s2->ind[1],
                     s2->ind[2], s2->ind[3]);
    set_sampler2D_uniform("tex2", v2);
    draw_unbind();
}

void subtract(GLuint add_program,
              const struct Vec4 *s1, frame_id v1,
              const struct Vec4 *s2, frame_id v2,
              frame_id res) {
    bind_quad(res, add_program);
    set_vec4_uniform("scale1",
                     s1->ind[0], s1->ind[1],
                     s1->ind[2], s1->ind[3]);
    set_sampler2D_uniform("tex1", v1);
    set_vec4_uniform("scale2",
                     -s2->ind[0], -s2->ind[1],
                     -s2->ind[2], -s2->ind[3]);
    set_sampler2D_uniform("tex2", v2);
    draw_unbind();
}
#include <stdio.h>

void explicit_part(const struct SimParams *params,
                   const struct Programs *programs, 
                   struct Frames *quads,
                   frame_id src1, frame_id src2, frame_id dest) {
    bind_quad(dest, programs->explicit_part);
    set_float_uniform("dt", params->dt);
    set_dx_dy_width_height(params->dx, params->dy,
                           params->width, params->height);
    set_sampler2D_uniform("waveTex0", src1);
    set_sampler2D_uniform("waveTex1", src2);
    set_sampler2D_uniform("dissipationTex", quads->dissipator);
    set_sampler2D_uniform("densityTex", quads->density);
    set_sampler2D_uniform("forceTex", quads->force);
    draw_unbind();
}

void implicit_part(const struct SimParams *params,
                   const struct Programs *programs, 
                   struct Frames *quads,
                   frame_id src, frame_id dest) {
    bind_quad(dest, programs->implicit_part);
    set_float_uniform("dt", params->dt);
    set_dx_dy_width_height(params->dx, params->dy,
                            params->width, params->height);
    
    set_sampler2D_uniform("waveTex", src);
    set_sampler2D_uniform("densityTex", quads->density);
    set_sampler2D_uniform("dissipationTex", quads->dissipator);
    draw_unbind();
}

struct ImplicitTransformData {
    const struct SimParams *params;
    const struct Programs *programs;
    struct Frames *quads;
};

void implicit_transform(frame_id y0, void *void_data, frame_id x) {
    struct ImplicitTransformData *data
         = (struct ImplicitTransformData *)void_data;
    const struct SimParams *params = data->params;
    const struct Programs *programs = data->programs;
    struct Frames *quads = data->quads;
    implicit_part(params, programs, quads, x, y0);
}


/*
h = b
g = b
r2 = g dot g
while r2 is not small enough DO
    k = A h
    lambda = r2 / (h dot k)
    g = g - lambda k
    r2' = g dot g
    gamma = r2' / r2
    x = x + lambda h
    h = g + gamma h
    r2 = r2'
end while
*/
void timestep(const struct SimParams *params,
              const struct Programs *programs, struct Frames *quads) {
    float t = params->t;
    float f = 1.0/30.0;
    bind_quad(quads->force, programs->init_dist);
    set_vec4_uniform("amplitude", 0.1*cos(2.0*PI*f*t), 
                     0.1*cos(2.0*PI*f*t), 0.1*cos(2.0*PI*f*t), 0.0);
    set_float_uniform("sigma_x", 0.01);
    set_float_uniform("sigma_y", 0.01);
    set_float_uniform("u0", 0.75);
    set_float_uniform("v0", 0.75);
    draw_unbind();
    struct CGController cg_controls = {.conj_quads=quads->cg,
                                       .sum_quads=quads->summations,
                                       .size=params->texel_width,
                                       .copy_program=programs->copy,
                                       .scale_program=programs->scale,
                                       .add_program=programs->add2,
                                       .multiply_program=programs->multiply,
                                       .epsilon.x=1e-10, .epsilon.y=1e-10,
                                       .epsilon.z=1e-10, .epsilon.w=1e-10,
                                       .min_iter=1, .max_iter=10};
    struct ImplicitTransformData transform_data = {.params=params,
                                                   .programs=programs,
                                                   .quads=quads};
    explicit_part(params, programs, quads,
                  quads->waves[0], quads->waves[1], quads->waves[2]);
    struct CGInfo cg_info = conjugate_gradient(&cg_controls,
                                               &implicit_transform,
                                               (void *)&transform_data,
                                               quads->waves[1],
                                               quads->waves[2]);
    printf("%d\n", cg_info.number_of_iterations);
    bind_quad(quads->waves[2], programs->copy);
    set_sampler2D_uniform("tex", cg_info.result);
    draw_unbind();
    roll3(&quads->waves[0], &quads->waves[1], &quads->waves[2]);
}

