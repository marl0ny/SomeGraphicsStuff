#include "gl_wrappers.hpp"
#include "parameters.hpp"
#include "tex_sort.hpp"


#ifndef _SIMULATION_
#define _SIMULATION_

using namespace sim_2d;

struct Frames {
    TextureParams default_tex_params;
    TextureParams position_velocities_tex_params;
    TextureParams forces_tex_params;
    TextureParams energies_tex_params;
    TextureParams cell_tex_params;
    // Quad data, tmp;
    Quad cells;
    Quad position_velocities[2];
    Quad forces[2];
    Quad energies;
    RenderTarget render_tmp;
    RenderTarget render;
    WireFrame quad_wire_frame;
    WireFrame particles_wire_frame;
    WireFrame container_wire_frame_inner;
    WireFrame container_wire_frame_outer;
    Frames(const TextureParams &default_tex_params, const SimParams &params);
    void reset_number_of_particles(const SimParams &params);
};

struct Programs {
    unsigned int copy;
    unsigned int copy_scale;
    unsigned int particles_view;
    unsigned int container_view;
    unsigned int uniform_linear_transform;
    unsigned int forces;
    unsigned int energies;
    struct {
        unsigned int positions, velocities1, velocities2;
    } verlet;
    Programs();
};

struct ConstructCellsGridRes {
    int max_cell_count;
    Vec2 max_velocity;
};

/* Molecular dynamics simulation, where the dynamics
are mostly computed in the GPU via GLSL shaders. The simulation
domain is partitioned into a grid of cells, which is used
to sort the position of the particles. To improve computational
efficiency, for each particle, only interactions
with other particle in its own cell or in neighbouring
cells are computed.  

References:

D. Schroeder. "Interactive Molecular Dynamics."
Available: https://physics.weber.edu/schroeder/md/.

D. Schroeder, "Interactive Molecular Dynamics,"
American Journal of Physics, 83, 3, 210 - 218. 2015.
http://dx.doi.org/10.1119/1.4901185

*/
class Simulation {
    Programs m_programs;
    Frames m_frames;
    sort::Sort m_sort;
    std::vector<Vec2> m_cell_array;
    std::vector<Vec2> m_forces_vec;
    std::vector<float> m_energies_vec;
    std::vector<Vec4> m_positions_velocities_vec;
    Vec2 m_max_velocity;
    float m_energy;
    float m_energy_start;
    Quad *m_position_velocities[2];
    Quad *m_forces[2];
    int m_counts_per_cell;
    float m_dt;
    void init_particles(
        IVec2 particles_per_cell_dimension,
        int left_cell_offset, int bottom_cell_offset,
        int right_cell_offset, int top_cell_offset,
        const SimParams &params);
    void init_box_particles(
        Vec2 r0, Vec2 v0, float spacing,
        float angular_offset, const SimParams &params);
    ConstructCellsGridRes construct_cells_grid(
        Quad &cells, Quad &positions,
        const SimParams &params);
    void compute_forces(
        Quad &forces, const Quad &positions, const Quad &cells,
        int max_particle_count_per_cell, 
        const SimParams &params);
    Vec2 find_max_force(const Quad &forces);
    void compute_energies(
        Quad &energies, const Quad &position_velocities, const Quad &cells,
        int max_particle_count_per_cell, 
        const SimParams &params);
    float sum_energies(const Quad &energies, const SimParams &params);
    public:
    Simulation(const TextureParams &default_tex_params,
               const SimParams &params);
    float time_step(const SimParams &params);
    const RenderTarget &view(SimParams &params, float scale, Vec2 translate);
    void reset_number_of_particles(const SimParams &params);
    float get_actual_time_step() const;
    float get_energy();
    float get_energy_drift_error();
    float get_energy_drift_error_percentage();
};

#endif