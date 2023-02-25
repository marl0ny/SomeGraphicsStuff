#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "gl_wrappers/gl_wrappers.h"
#include "init_render.h"
#include "cg_gl.h"
#include "fft_gl.h"

struct Programs {
    GLuint zero, copy, scale, add2, multiply;
    GLuint complex_multiply;
    GLuint charge_density;
    GLuint view_particles;
    GLuint rev_bit_sort2;
    GLuint fft_iter;
    GLuint laplacian;
    GLuint uniform_colour;
    GLuint ft_poisson;
    GLuint ft_gradient;
    GLuint sample_field;
    GLuint view_particle_vectors;
    GLuint timestep;
    GLuint enforce_periodic;
};

struct SimParams {
    int view_width, view_height;
    int texel_width, texel_height;
    int particles_count;
    int particles_texel_width;
    int particles_texel_height;
    int number_of_steps;
    struct {
        int sizeof_vertices;
        int sizeof_elements;
    } particles_view;
    struct {
        int sizeof_vertices;
        int sizeof_elements;
    } particle_vectors_view;
    int use_cg;
    float dt;
    float m;
};

#define FRAMES_SUB_VIEWS_COUNT 3
#define FRAMES_SUMMATIONS_COUNT 10
#define FRAMES_CG_COUNT 11
#define FRAMES_POTENTIALS_COUNT 2
#define FRAMES_FFT_ITERS_COUNT 2
#define FRAMES_EXTRAS_POTENTIAL_COUNT 3
#define FRAMES_PARTICLES_COUNT 3
#define FRAMES_EXTRAS_PARTICLES_COUNT 2
struct Frames {
    frame_id main_view;
    frame_id sub_views[FRAMES_SUB_VIEWS_COUNT];
    frame_id particles_view;
    // Meant for viewing vector quantities like the
    // velocity or force of each particle.
    frame_id particle_vectors_view;
    frame_id summations[FRAMES_SUMMATIONS_COUNT];
    frame_id cg[FRAMES_CG_COUNT];
    frame_id charge_density;
    frame_id potentials[FRAMES_POTENTIALS_COUNT];
    frame_id fft_iters[FRAMES_FFT_ITERS_COUNT];
    // Fourier transform of the charge density
    frame_id ft_charge_density;
    // Fourier transform of the potential
    frame_id ft_potential;
    // Gradient of the potential in frequency space
    frame_id ft_gradient_potential;
    frame_id gradient_potential;
    // Gradient of the potential at each particle location
    frame_id particles_gradient_potential;
    frame_id extras_potential[FRAMES_EXTRAS_POTENTIAL_COUNT];
    frame_id particles[FRAMES_PARTICLES_COUNT];
    frame_id particles_extras[FRAMES_EXTRAS_PARTICLES_COUNT];
    frame_id draw;
};

typedef struct Programs Programs;
typedef struct SimParams SimParams;
typedef struct Frames Frames;


Programs s_programs = {};
SimParams s_sim_params = {};
Frames s_frames = {};

void init_programs(Programs *programs) {
    programs->zero = make_quad_program("./shaders/zero.frag");
    programs->copy = make_quad_program("./shaders/copy.frag");
    programs->scale = make_quad_program("./shaders/scale.frag");
    programs->uniform_colour
        = make_quad_program("./shaders/uniform-colour.frag");
    programs->multiply = make_quad_program("./shaders/multiply.frag");
    programs->complex_multiply
        = make_quad_program("./shaders/complex-multiply.frag");
    programs->add2 = make_quad_program("./shaders/add2.frag");
    programs->copy = make_quad_program("./shaders/copy.frag");
    programs->charge_density
        = make_program("./shaders/charge-density.vert",
                       "./shaders/charge-density.frag");
    programs->view_particles
        = make_program("./shaders/view-particles.vert",
                       "./shaders/view-particles.frag");
    programs->laplacian = make_quad_program("./shaders/laplacian.frag");
    programs->fft_iter = make_quad_program("./shaders/fft-iter.frag");
    programs->rev_bit_sort2
        = make_quad_program("./shaders/rev-bit-sort2.frag");
    programs->ft_poisson
        = make_quad_program("./shaders/ft-poisson.frag");
    programs->ft_gradient
        = make_quad_program("./shaders/ft-gradient.frag");
    programs->sample_field
        = make_quad_program("./shaders/sample-field.frag");
    programs->view_particle_vectors
        = make_program("./shaders/view-particle-vectors.vert",
                       "./shaders/view-particle-vectors.frag");
    programs->timestep
        = make_quad_program("./shaders/timestep.frag");
    programs->enforce_periodic
        = make_quad_program("./shaders/enforce-periodic.frag");
}

void init_sim_params(SimParams *params) {
    #ifdef __APPLE__
    params->view_width = 1024;
    params->view_height = 1024;
    #else
    params->view_width = 512;
    params->view_height = 512;
    #endif
    params->texel_width = 512;
    params->texel_height = 512;
    params->particles_texel_width = 128;
    params->particles_texel_height = 128;
    params->particles_count
        = params->particles_texel_width*params->particles_texel_height;
    params->use_cg = 0;
    params->dt = 0.01;
    params->m = 100.0;
    params->number_of_steps = 0;
}

void init_particle_positions(Frames *frames, Programs *programs,
                             SimParams *params) {
    int particles_count = params->particles_count;
    glViewport(0, 0, params->particles_texel_width,
               params->particles_texel_height);
    struct Vec4 *pos_vel = malloc(particles_count*
                                  sizeof(struct Vec4));
    for (int i = 0; i < particles_count; i++) {
        float r = 0.7*(float)rand()/(float)RAND_MAX;
        float tau = 2.0*3.14159;
        float theta = tau*(float)rand()/(float)RAND_MAX;
        pos_vel[i].ind[0] = r*cos(theta);
        pos_vel[i].ind[1] = r*sin(theta);
        pos_vel[i].ind[2] = 3.7*r*sin(theta);
        pos_vel[i].ind[3] = -3.7*r*cos(theta);
    }
    for (int i = 0; i < FRAMES_PARTICLES_COUNT; i++) {
        bind_quad(frames->particles[i], programs->zero);
        draw_unbind_quad();
        quad_substitute_array(frames->particles[i],
                              params->particles_texel_width,
                              params->particles_texel_height,
                              GL_FLOAT, (void *)pos_vel);
    }
    free(pos_vel);
}

void init_particles_view_frame(frame_id *particles_view_frame,
                               struct TextureParams *tex_params,
                               SimParams *params) {
    params->particles_view.sizeof_elements = 0;
    params->particles_view.sizeof_vertices
        = params->particles_count*sizeof(struct Vec4);
    struct Vec4 *v = malloc(params->particles_view.sizeof_vertices);
    for (int i = 0; i < params->particles_texel_height; i++) {
        for (int j = 0; j < params->particles_texel_width; j++) {
            v[i*params->particles_texel_width + j].ind[0]
                = ((float)j + 0.5)/params->particles_texel_width;
            v[i*params->particles_texel_width + j].ind[1]
                = ((float)i + 0.5)/params->particles_texel_height;
            v[i*params->particles_texel_width + j].ind[2] = 0.0;
            v[i*params->particles_texel_width + j].ind[3] = 1.0;
        }
    }
    *particles_view_frame = new_frame(tex_params, (float *)v,
        params->particles_view.sizeof_vertices, NULL, -1);
    free(v);
}

void init_particle_vectors_view_frame(frame_id *view_frame,
                                      struct TextureParams *tex_params,
                                      SimParams *params) {
    params->particle_vectors_view.sizeof_elements = 0;
    params->particle_vectors_view.sizeof_vertices
        = 2*params->particles_count*sizeof(struct Vec4);
    struct Vec4 *v = malloc(
        params->particle_vectors_view.sizeof_vertices);
    for (int i = 0; i < params->particles_texel_height; i++) {
        for (int j = 0; j < params->particles_texel_width; j++) {
            for (int k = 0; k < 2; k++) {
                v[2*(i*params->particles_texel_width + j) + k].ind[0]
                    = ((float)j + 0.5)/params->particles_texel_width;
                v[2*(i*params->particles_texel_width + j) + k].ind[1]
                    = ((float)i + 0.5)/params->particles_texel_height;
                v[2*(i*params->particles_texel_width + j) + k].ind[2]
                    = 0.0;
                v[2*(i*params->particles_texel_width + j) + k].ind[3]
                    = (k == 0)? 0.0: 1.0;
            }
        }
    }
    *view_frame = new_frame(tex_params, (float *)v,
                            params->particles_view.sizeof_vertices,
                            NULL, -1);
    free(v);
}

frame_id init_charge_density_frame(struct TextureParams *tex_params,
                                  SimParams *params) {
    frame_id charge_density_frame;
    init_particles_view_frame(&charge_density_frame, tex_params, params);
    return charge_density_frame;
}


void fft(frame_id dst, frame_id src, int is_inverse) {
    rev_bit_sort2(s_programs.rev_bit_sort2, src, s_frames.fft_iters[0],
                  s_sim_params.texel_width, s_sim_params.texel_height);
    int size = s_sim_params.texel_width;
    frame_id res;
    if (is_inverse) {
        res = ifft_iter(s_programs.fft_iter,
                        s_frames.fft_iters[0], s_frames.fft_iters[1],
                        size, 0);
    } else {
        res = fft_iter(s_programs.fft_iter,
                       s_frames.fft_iters[0], s_frames.fft_iters[1],
                       size, 0);
    }
    if (res != s_frames.fft_iters[0]) {
        frame_id tmp = s_frames.fft_iters[0];
        s_frames.fft_iters[0] = s_frames.fft_iters[1];
        s_frames.fft_iters[1] = tmp;
    }
    if (is_inverse) {
        res = ifft_iter(s_programs.fft_iter,
                        s_frames.fft_iters[0], s_frames.fft_iters[1],
                        size, 1);
    } else {
        res = fft_iter(s_programs.fft_iter,
                       s_frames.fft_iters[0], s_frames.fft_iters[1],
                       size, 1);
    }
    bind_quad(dst, s_programs.copy);
    set_sampler2D_uniform("tex", res);
    draw_unbind_quad();
}

void init_frames(Frames *frames, SimParams *params) {
    frames->main_view = new_quad(NULL);
    struct TextureParams tex_params = {
        .type=GL_FLOAT,
        .width=params->view_width,
        .height=params->view_height,
        .generate_mipmap=1,
        .wrap_s=GL_CLAMP_TO_EDGE, .wrap_t=GL_CLAMP_TO_EDGE,
        .min_filter=GL_LINEAR, .mag_filter=GL_LINEAR
    };
    for (int i = 0; i < FRAMES_SUB_VIEWS_COUNT; i++) {
        frames->sub_views[i] = new_quad(&tex_params);
    }
    init_particles_view_frame(&frames->particles_view, &tex_params,
                              params);
    init_particle_vectors_view_frame(&frames->particle_vectors_view,
                                     &tex_params, params);

    // Make the frames that are used for computing the potential.
    tex_params.width = params->texel_width;
    tex_params.height = params->texel_height;
    tex_params.wrap_s = GL_REPEAT;
    tex_params.wrap_t = GL_REPEAT;
    frames->charge_density
        = init_charge_density_frame(&tex_params, params);
    for (int i = 0; i < FRAMES_POTENTIALS_COUNT; i++) {
        frames->potentials[i] = new_quad(&tex_params);
    }
    for (int i = 0; i < FRAMES_FFT_ITERS_COUNT; i++) {
        frames->fft_iters[i] = new_quad(&tex_params);
    }
    for (int i = 0; i < FRAMES_EXTRAS_POTENTIAL_COUNT; i++) {
        frames->extras_potential[i] = new_quad(&tex_params);
    }
    for (int i = 0; i < FRAMES_CG_COUNT; i++) {
        frames->cg[i] = new_quad(&tex_params);
    }
    frames->ft_charge_density = new_quad(&tex_params);
    frames->ft_potential = new_quad(&tex_params);
    frames->ft_gradient_potential = new_quad(&tex_params);
    frames->gradient_potential = new_quad(&tex_params);
    for (int i = 0, size = params->texel_width/2;
         i < FRAMES_SUMMATIONS_COUNT; i++, size /= 2) {
        tex_params.width = size;
        tex_params.height = size;
        frames->summations[i] = new_quad(&tex_params);
    }

    // Initialize frames for storing particle positions and velocities.
    tex_params.width = params->particles_texel_width;
    tex_params.height = params->particles_texel_height;
    tex_params.wrap_s = GL_CLAMP_TO_EDGE;
    tex_params.wrap_t = GL_CLAMP_TO_EDGE;
    for (int i = 0; i < FRAMES_PARTICLES_COUNT; i++) {
        frames->particles[i] = new_quad(&tex_params);
    }
    for (int i = 0; i < FRAMES_EXTRAS_PARTICLES_COUNT; i++) {
        frames->particles_extras[i] = new_quad(&tex_params);
    }
    frames->particles_gradient_potential = new_quad(&tex_params);
    frames->draw = new_quad(&tex_params);
}

void timestep() {
    bind_quad(s_frames.particles_extras[0], s_programs.timestep);
    set_float_uniform("dt", s_sim_params.dt);
    set_float_uniform("m",  s_sim_params.m);
    set_int_uniform("gradIsComplex", 1);
    set_sampler2D_uniform("texParticles0", s_frames.particles[0]);
    frame_id particles1 = (s_sim_params.number_of_steps == 0)?
        s_frames.particles[0]: s_frames.particles[1];
    set_sampler2D_uniform("texParticles1", particles1);
    set_sampler2D_uniform("texGradPotential",
                          s_frames.particles_gradient_potential);
    draw_unbind_quad();

    if (s_sim_params.number_of_steps == 0)
        bind_quad(s_frames.particles[1], s_programs.enforce_periodic);
    else
        bind_quad(s_frames.particles[2], s_programs.enforce_periodic);
    set_sampler2D_uniform("posVelTex", s_frames.particles_extras[0]);
    set_vec2_uniform("verticalBoundaries", -1.0, 1.0);
    set_vec2_uniform("horizontalBoundaries", -1.0, 1.0);
    draw_unbind_quad();
}

void init() {
    init_programs(&s_programs);
    init_sim_params(&s_sim_params);
    init_frames(&s_frames, &s_sim_params);
    init_particle_positions(&s_frames, &s_programs, &s_sim_params);
}


struct TransformData {
    GLuint laplacian_program;
    GLuint scale_program;
    SimParams *params;
    frame_id tmp;
};

void transform(frame_id y, void *voided_transform_data, frame_id x) {
    struct TransformData *transform_data
        = (struct TransformData *)voided_transform_data;
    SimParams *params = transform_data->params;
    bind_quad(transform_data->tmp, transform_data->laplacian_program);
    set_sampler2D_uniform("tex", x);
    set_float_uniform("width", (float)params->texel_width);
    set_float_uniform("height", (float)params->texel_height);
    set_float_uniform("dx", 1.0);
    set_float_uniform("dy", 1.0);
    set_int_uniform("orderOfAccuracy", 2);
    draw_unbind_quad();
    bind_quad(y, transform_data->scale_program);
    set_float_uniform("scale", -1.0);
    set_sampler2D_uniform("tex", transform_data->tmp);
    draw_unbind_quad();
}

frame_id compute_potential(frame_id charge_density) {
    frame_id result;
    if (s_sim_params.use_cg) {
        struct CGController controls = {
            .conj_quads = &s_frames.cg[0],
            .sum_quads = &s_frames.summations[0],
            .size = s_sim_params.texel_width,
            .copy_program = s_programs.copy,
            .scale_program = s_programs.scale,
            .add_program = s_programs.add2,
            .multiply_program = s_programs.multiply,
            .epsilon.ind[0] = 1e-5,
            .epsilon.ind[1] = 1e-5,
            .epsilon.ind[2] = 1e-5,
            .epsilon.ind[3] = 1e-5,
            .min_iter = 5,
            .max_iter = 10,
        };
        struct TransformData transform_data = {
            .laplacian_program = s_programs.laplacian,
            .scale_program = s_programs.scale,
            .params = &s_sim_params,
            .tmp = s_frames.extras_potential[0],
        };
        struct CGInfo cg_info = conjugate_gradient(&controls,
                                                transform,
                                                (void *)&transform_data,
                                                s_frames.potentials[0],
                                                charge_density);
        result = cg_info.result;
    } else {
        fft(s_frames.ft_charge_density, charge_density, 0);
        bind_quad(s_frames.ft_potential, s_programs.ft_poisson);
        set_int_uniform("texelWidth", s_sim_params.texel_width);
        set_int_uniform("texelHeight", s_sim_params.texel_height);
        set_sampler2D_uniform("densityTex", s_frames.ft_charge_density);
        set_vec4_uniform("constant", 100.0, 0.0, 100.0, 0.0);
        draw_unbind_quad();
        fft(s_frames.potentials[0], s_frames.ft_potential, 1);
        result = s_frames.potentials[0];
    }
    return result;
}

static void swap3(frame_id *f0, frame_id *f1, frame_id *f2) {
    frame_id tmp0 = *f1, tmp1 = *f2, tmp2 = *f0;
    *f0 = tmp0, *f1 = tmp1, *f2 = tmp2;
}


void render(struct RenderParams *render_params) {
    int texel_width = s_sim_params.texel_width;
    int texel_height = s_sim_params.texel_height;
    int view_width = s_sim_params.view_width;
    int view_height = s_sim_params.view_height;
    glViewport(0, 0, texel_width, texel_height);

    bind_quad(s_frames.draw, s_programs.uniform_colour);
    set_vec3_uniform("colour", 1.0, 0.5, 0.5);
    draw_unbind_quad();

    struct VertexParam vertex_params[1] = {
        {.name="particleTexCoord", .size=4,
         .type=GL_FLOAT, .normalized=GL_FALSE,
         .stride=4*sizeof(float), .offset=0},
    };

    frame_id particles1 = (s_sim_params.number_of_steps == 0)?
        s_frames.particles[0]: s_frames.particles[1];

    // Charge density
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    bind_frame(s_frames.charge_density, s_programs.charge_density);
    set_vertex_attributes(vertex_params, 1);
    set_sampler2D_uniform("tex", particles1);
    glDrawArrays(GL_POINTS, 0, s_sim_params.particles_count);
    unbind();
    glDisable(GL_BLEND);

    frame_id result = compute_potential(s_frames.charge_density);

    if (!s_sim_params.use_cg) {
        bind_quad(s_frames.ft_gradient_potential, s_programs.ft_gradient);
        set_int_uniform("whichComponent", 0);
        set_int_uniform("texelWidth", s_sim_params.texel_width);
        set_int_uniform("texelHeight", s_sim_params.texel_height);
        set_sampler2D_uniform("tex", s_frames.ft_potential);
        draw_unbind_quad();
        fft(s_frames.gradient_potential,
            s_frames.ft_gradient_potential, 1);
    }

    glViewport(0, 0, s_sim_params.particles_texel_width,
               s_sim_params.particles_texel_height);
    bind_quad(s_frames.particles_gradient_potential,
              s_programs.sample_field);
    set_sampler2D_uniform("posVelTex", particles1);
    set_sampler2D_uniform("fieldTex", s_frames.gradient_potential);
    draw_unbind_quad();
    timestep();

    glViewport(0, 0, view_width, view_height);

    bind_frame(s_frames.particles_view, s_programs.view_particles);
    set_vertex_attributes(vertex_params, 1);
    set_sampler2D_uniform("tex", particles1);
    glDrawArrays(GL_POINTS, 0, s_sim_params.particles_count);
    unbind();

    bind_frame(s_frames.particle_vectors_view,
               s_programs.view_particle_vectors);
    set_vertex_attributes(vertex_params, 1);
    set_sampler2D_uniform("posVelTex", particles1);
    set_sampler2D_uniform("vecTex", 
                          particles1);
    // set_float_uniform("vecScale", 0.1*0.001);
    set_float_uniform("vecScale", 0.01);
    set_ivec2_uniform("vecIndex", 1, 2);
    glDrawArrays(GL_LINES, 0, s_sim_params.particles_count);
    unbind();

    bind_quad(s_frames.main_view, s_programs.scale);
    set_float_uniform("scale", 1.0);
    set_sampler2D_uniform("tex", s_frames.particle_vectors_view);
    draw_unbind_quad();

    if (s_sim_params.number_of_steps != 0)
        swap3(&s_frames.particles[0], &s_frames.particles[1], 
              &s_frames.particles[2]);

    s_sim_params.number_of_steps++;
}
