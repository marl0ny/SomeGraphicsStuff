#include "gl_wrappers/gl_wrappers.h"


#ifdef __cplusplus
//extern "C" {
#endif

#ifndef _SIMULATION_
#define _SIMULATION_

#define SQUARE_SIDE_LENGTH 512
#define MAX_WIDTH 512
#define MAX_HEIGHT 512
#define START_WIDTH SQUARE_SIDE_LENGTH
#define START_HEIGHT SQUARE_SIDE_LENGTH


struct SimParams {
    int steps_per_frame;
    int texel_width, texel_height;
    float dt;
    float t;
    float width, height;
    float dx, dy;
    struct {
        struct Vec4 amplitude;
        float sigma_x, sigma_y;
        float u0, v0;
    } init_wave;
};

struct Programs {
    GLuint zero, copy, resize_copy, copy2, scale, add2, multiply;
    GLuint init_dist;
    GLuint explicit_part, implicit_part;
    GLuint view;
};

#define N_SIM_FRAMES 6
#define N_ITER_FRAMES 11
#define SUM_FRAMES 8

struct Frames {
    union {
        struct {
            frame_id main_view;
            frame_id secondary_view;
            frame_id dissipator;
            frame_id density;
            frame_id force;
            frame_id waves[3];
            frame_id cg[N_ITER_FRAMES];
            frame_id summation_iter8[SUM_FRAMES];
        };
        struct {
            frame_id views[2];
            frame_id simulation[N_SIM_FRAMES];
            frame_id iter_solver[N_ITER_FRAMES];
            frame_id summations[SUM_FRAMES];
        };
    };
};


void init_sim_params(struct SimParams *params);

void init_programs(struct Programs *programs);

void init_frames(struct Frames *quads, const struct SimParams *params);

void set_dx_dy_width_height(float dx, float dy, float width, float height);

struct Vec4 dot(frame_id multiply_program, frame_id scale_program,
                frame_id *sum_quads, int size,
                frame_id v1, frame_id v2, frame_id v1_v2);

void add(GLuint add_program,
         const struct Vec4 *s1, frame_id v1,
         const struct Vec4 *s2, frame_id v2,
         frame_id res);

void subtract(GLuint add_program,
              const struct Vec4 *s1, frame_id v1,
              const struct Vec4 *s2, frame_id v2,
              frame_id res);

void timestep(const struct SimParams *params,
              const struct Programs *programs, struct Frames *quads);

#endif

#ifdef __cplusplus
// }
#endif
