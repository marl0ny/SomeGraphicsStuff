#include "gl_wrappers/gl_wrappers.h"

#ifndef _SIMULATION_H_
#define _SIMULATION_H_

struct SimPrograms {
    int zero, copy, scale;
    int complex_scale;
    int complex_add2;
    int add5;
    int init;
    int kinetic;
};

struct SimFrames {
    frame_id potential_psi[5];
    frame_id temps[5];
    frame_id summations[7];
};

struct SimParams {
    float dt;
    int step_count;
    float dx, dy, width, height;
    int texel_width, texel_height;
    float sigma, kappa;
    struct Vec2 complex_psi_step_offset;
};

typedef struct SimPrograms SimPrograms;
typedef struct SimFrames SimFrames;
typedef struct SimParams SimParams;

void init_sim_programs(SimPrograms *programs);

void init_sim_frames(SimFrames *frames, const SimParams *params);

void init_sim_params(SimParams *params);

void normalize(SimFrames *frames, SimPrograms *programs,
               SimParams *params);

void init_potential_psi(SimFrames *frames, SimPrograms *programs,
                        SimParams *params);

void timestep_forward_euler(SimFrames *frames,
                            SimPrograms *programs, SimParams *params);

void timestep_rk4(SimFrames *frames, SimPrograms *programs,
                  SimParams *params);

void timestep_schrod(SimFrames *frames, SimPrograms *programs,
                    SimParams *params);

#endif
