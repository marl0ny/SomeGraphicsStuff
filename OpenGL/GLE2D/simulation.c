#include "simulation.h"
#include "gl_wrappers/gl_wrappers.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


void init_sim_params(SimParams *params) {
    params->dt = 0.003;
    params->texel_width = 256;
    params->texel_height = 256;
    params->width = 64.0;
    params->height = 64.0;
    params->dx = params->width/params->texel_width;
    params->dy = params->height/params->texel_height;
    params->sigma = 5e1;
    params->kappa = 300000.0;
    // params->complex_psi_step_offset.x = 0.07; // 1.0/sqrt(2.0);
    // params->complex_psi_step_offset.y = 1.0; // 1.0/sqrt(2.0);
    params->complex_psi_step_offset.x = 1.0; // 1.0/sqrt(2.0);
    params->complex_psi_step_offset.y = 0.0; // 1.0/sqrt(2.0);
    params->step_count = 0;
}

void init_sim_programs(SimPrograms *programs) {
    programs->copy = make_quad_program("./shaders/copy.frag");
    programs->scale = make_quad_program("./shaders/scale.frag");
    programs->zero = make_quad_program("./shaders/zero.frag");
    programs->complex_scale
        = make_quad_program("./shaders/complex-scale.frag");
    programs->kinetic = make_quad_program("./shaders/kinetic.frag");
    programs->init = make_quad_program("./shaders/init.frag");
    programs->complex_add2 = make_quad_program("./shaders/complex-add2.frag");
    programs->add5 = make_quad_program("./shaders/add5.frag");
}

void init_sim_frames(SimFrames *frames, const SimParams *params) {
    struct TextureParams tex_params = {
        .type=GL_FLOAT,
        .width=params->texel_width,
        .height=params->texel_height,
        .generate_mipmap=1,
        .wrap_s=GL_REPEAT, .wrap_t=GL_REPEAT,
        .min_filter=GL_LINEAR, .mag_filter=GL_LINEAR,
    };
    for (int i = 0; i < 5; i++) {
        frames->potential_psi[i] = new_quad(&tex_params);
        frames->temps[i] = new_quad(&tex_params);
    }
    tex_params.wrap_s = GL_REPEAT;
    tex_params.wrap_t = GL_REPEAT;
    for (int i = 0; i < 7; i++) {
        frames->summations[i] = new_quad(&tex_params);
        tex_params.width /= 2;
        tex_params.height /= 2;
    }
}

static void set_dx_dy_w_h(const SimParams *params) {
    set_float_uniform("dx", params->dx);
    set_float_uniform("dy", params->dy);
    set_float_uniform("w", params->width);
    set_float_uniform("h", params->height);
}

void normalize(SimFrames *frames, SimPrograms *programs,
               SimParams *params) {
    bind_quad(frames->potential_psi[1], programs->copy);
    set_sampler2D_uniform("tex", frames->potential_psi[0]);
    draw_unbind_quad();
    int size = params->texel_width*params->texel_height;
    struct Vec4 *arr = malloc(size*sizeof(struct Vec4));
    get_quad_texture_array(frames->potential_psi[1],
                           0, 0, params->texel_width, params->texel_height,
                           GL_FLOAT, (float *)arr);
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i].z*arr[i].z + arr[i].w*arr[i].w;
    }
    printf("%g\n", sum);
    bind_quad(frames->potential_psi[0], programs->scale);
    set_vec4_uniform("scale", 1.0, 1.0,
                    1.0/sqrt(sum),
                    1.0/sqrt(sum));
    set_sampler2D_uniform("tex", frames->potential_psi[1]);
    draw_unbind_quad();
    free(arr);
}

void init_potential_psi(SimFrames *frames, SimPrograms *programs,
                        SimParams *params) {
    bind_quad(frames->potential_psi[0], programs->init);
    set_float_uniform("amplitude", 5.0);
    set_float_uniform("sigma", 0.08);
    set_vec2_uniform("r0", 0.4, 0.4);
    set_ivec2_uniform("wavenumber", 15, -30);
    set_int_uniform("type", 0);
    draw_unbind_quad();
    normalize(frames, programs, params);
}

static void swap3(frame_id *f0, frame_id *f1, frame_id *f2) {
    frame_id tmp0 = *f0, tmp1 = *f1, tmp2 = *f2;
    *f0 = tmp1, *f1 = tmp2, *f2 = tmp0;
}

void eom(const SimFrames *frames,
         const SimPrograms *programs,
         const SimParams *params,
         frame_id dst, frame_id src) {
    bind_quad(dst, programs->kinetic);
    set_dx_dy_w_h(params);
    set_sampler2D_uniform("tex", src);
    set_float_uniform("sigma", params->sigma);
    set_float_uniform("kappa", params->kappa);
    set_vec2_uniform("offsetA", 0.0, 0.0);
    set_vec2_uniform("complexPsiStepOffset",
                     params->complex_psi_step_offset.x,
                     params->complex_psi_step_offset.y);
    draw_unbind_quad();
}

static void re_add2_frames(const SimFrames *frames,
                           const SimPrograms *programs,
                           const SimParams *params,
                           frame_id dst,
                           double s1, frame_id src1,
                           double s2, frame_id src2) {
    bind_quad(dst, programs->complex_add2);
    set_vec4_uniform("scale1", s1, 0.0, s1, 0.0);
    set_sampler2D_uniform("tex1", src1);
    set_vec4_uniform("scale2", s2, 0.0, s2, 0.0);
    set_sampler2D_uniform("tex2", src2);
    draw_unbind_quad();
}

void timestep_forward_euler(SimFrames *frames,
                            SimPrograms *programs, SimParams *params) {
    float dt = params->dt;
    #define CONST_INPUTS frames, programs, params
    eom(CONST_INPUTS,
        frames->potential_psi[1], frames->potential_psi[0]);
    #undef CONST_INPUTS
    bind_quad(frames->potential_psi[0], programs->copy);
    set_sampler2D_uniform("tex", frames->potential_psi[1]);
    draw_unbind_quad();
    params->step_count++;
}

void timestep_rk4(SimFrames *frames,
                  SimPrograms *programs, SimParams *params) {
    float dt = params->dt;
    frame_id sum_frame = frames->temps[0];

    #define CONST_INPUTS frames, programs, params

    eom(CONST_INPUTS, frames->potential_psi[1], frames->potential_psi[0]);

    re_add2_frames(CONST_INPUTS, sum_frame,
                   1.0, frames->potential_psi[0],
                   0.5*dt, frames->potential_psi[1]);
    eom(CONST_INPUTS, frames->potential_psi[2], sum_frame);

    re_add2_frames(CONST_INPUTS, sum_frame,
                   1.0, frames->potential_psi[0],
                   0.5*dt, frames->potential_psi[2]);
    eom(CONST_INPUTS, frames->potential_psi[3], sum_frame);

    re_add2_frames(CONST_INPUTS, sum_frame,
                   1.0, frames->potential_psi[0],
                   dt, frames->potential_psi[3]);
    eom(CONST_INPUTS, frames->potential_psi[4], sum_frame);

    #undef CONST_INPUTS

    bind_quad(sum_frame, programs->add5);
    set_float_uniform("s0", 1.0);
    set_sampler2D_uniform("tex0", frames->potential_psi[0]);
    set_float_uniform("s1", dt/6.0);
    set_sampler2D_uniform("tex1", frames->potential_psi[1]);
    set_float_uniform("s2", dt/3.0);
    set_sampler2D_uniform("tex2", frames->potential_psi[2]);
    set_float_uniform("s3", dt/3.0);
    set_sampler2D_uniform("tex3", frames->potential_psi[3]);
    set_float_uniform("s4", dt/6.0);
    set_sampler2D_uniform("tex4", frames->potential_psi[4]);
    draw_unbind_quad();

    bind_quad(frames->potential_psi[0], programs->copy);
    set_sampler2D_uniform("tex", sum_frame);
    draw_unbind_quad();

    params->step_count++;
}

void timestep_schrod(SimFrames *frames, SimPrograms *programs,
                     SimParams *params) {
    float dt = (params->step_count == 0)? params->dt/2.0: params->dt;
    int frame0 = frames->potential_psi[0];
    int frame1 = (params->step_count == 0)?
        frames->potential_psi[0]: frames->potential_psi[1];
    int frame2 = (params->step_count == 0)?
        frames->potential_psi[1]: frames->potential_psi[2];
    eom(frames, programs, params, frames->temps[0], frame1);
    bind_quad(frame2, programs->complex_add2);
    set_vec4_uniform("scale1", dt/params->sigma, 0.0, 0.5*dt, 0.0);
    // set_vec4_uniform("scale1", 0.0, 0.0, 0.0, -0.5*dt);
    set_sampler2D_uniform("tex1", frames->temps[0]);
    set_vec4_uniform("scale2", 1.0, 0.0, 1.0, 0.0);
    set_sampler2D_uniform("tex2", frame0);
    draw_unbind_quad();
    if (params->step_count != 0)
        swap3(&frames->potential_psi[0],
              &frames->potential_psi[1], &frames->potential_psi[2]);
    params->step_count++;

}
