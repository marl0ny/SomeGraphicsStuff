#include "gl_wrappers.hpp"
#include "parameters.hpp"
#include "tex_sort.hpp"


#ifndef _SIMULATION_
#define _SIMULATION_

using namespace sim_2d;

struct Frames {
    TextureParams default_tex_params;
    TextureParams sim_tex_params;
    Quad data, tmp;
    RenderTarget render_tmp;
    RenderTarget render;
    WireFrame quad_wire_frame;
    Frames(const TextureParams &default_tex_params, const SimParams &params);
};

struct Programs {
    unsigned int copy;
    unsigned int sort4;
    struct bitonic_sort {
        unsigned int iter_part1;
        unsigned int iter_part2;
    };
    unsigned int user_defined;
    Programs();
};

class Simulation {
    Programs m_programs;
    Frames m_frames;
    sort::Sort m_sort;
    public:
    Simulation(const TextureParams &default_tex_params,
               const SimParams &params);
    const RenderTarget 
    &view(SimParams &params);
};

#endif