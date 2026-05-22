#include "gl_wrappers.hpp"
#include "parameters.hpp"
#include "volume_render.hpp"
#include "planar_slice.hpp"
#include "line_arrows3d.hpp"
#include "conical_arrow3d.hpp"

#ifndef _SIMULATION_
#define _SIMULATION_

using namespace sim_2d;

struct Frames {
    TextureParams data_tex_params;
    Quad data, tmp;
    RenderTarget render_tmp;
    RenderTarget render_tmp2;
    RenderTarget render;
    WireFrame quad_wire_frame;
    WireFrame arrows3d_frame;
    WireFrame conical_arrows3d_frame;
    Frames(const TextureParams &default_tex_params, const SimParams &params);
    void reset_data_dimensions(IVec3 texel_dimensions_3d);
};

struct Programs {
    unsigned int domain_color;
    unsigned int copy;
    unsigned int user_defined;
    unsigned int blur;
    // For arrows
    // unsigned int arrows3d;
    unsigned int gradient;
    unsigned int cube_outline;
    Programs();
};

class Simulation {
    volume_render::VolumeRender m_volume_render;
    planar_slice::PlanarSlices m_planar_slices;
    line_arrows3d::Arrows m_arrows3d;
    conical_arrows3d::Arrows m_conical_arrows3d;
    Programs m_programs;
    Frames m_frames;
    const RenderTarget
    &view_volume_render(
        SimParams &params, ::Quaternion rotation, float scale);
    const RenderTarget
    &view_planar_slices(
        SimParams &params, ::Quaternion rotation, float scale);
    public:
    Simulation(const TextureParams &default_tex_params,
               const SimParams &params);
    const RenderTarget 
    &view(SimParams &params,
        const std::optional<Vec2> &hover,
        ::Quaternion rotation, float scale);
    const RenderTarget &view_data_texture(
        SimParams &params, ::Quaternion rotation, float scale
    );
    const RenderTarget &view_volume_texture(
        SimParams &params, ::Quaternion rotation, float scale
    );
    void add_user_defined(
        const SimParams &params,
        unsigned int program,
        const std::map<std::string, float> &uniforms);
    void reset_data_dimensions(IVec3 texel_dimensions_3d);
    void reset_volume_dimensions(IVec3 volume_dimensions_3d);
    void reset_volume_filtering(unsigned int filtering);
};

#endif