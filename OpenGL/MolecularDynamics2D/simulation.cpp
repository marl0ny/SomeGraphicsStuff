#include "simulation.hpp"
#include "particles_wire_frame.hpp"
#include "tex_sort.hpp"
#include <cmath>
#include <random>

using namespace sim_2d;


static struct IVec2 decompose(unsigned int n) {
    struct IVec2 d = {.ind={(int)n, 1}};
    int i = 1;
    for (; i*i < n; i++) {}
    for (; n % i; i--) {}
    d.ind[0] = ((n / i) > i)? (n / i): i;
    d.ind[1] = ((n / i) < i)? (n / i): i;
    return d;
}

static const std::vector<float> QUAD_VERTICES = {
    -1.0, -1.0, 0.0, -1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, -1.0, 0.0};
static const std::vector<int> QUAD_ELEMENTS = {0, 1, 2, 0, 2, 3};
static WireFrame get_quad_wire_frame() {
    return WireFrame(
        {{"position", Attribute{
            3, GL_FLOAT, false,
            0, 0}}},
        QUAD_VERTICES, QUAD_ELEMENTS,
        WireFrame::TRIANGLES
    );
}

static WireFrame get_container_wire_frame_inner(const SimParams &params) {
    int cellWidth = params.cellGridDimensions2D[0];
    int cellHeight = params.cellGridDimensions2D[1];
    float x0 = 2.0/float(cellWidth), y0=2.0/float(cellHeight);
    float x1 = 1.0 - 2.0/float(cellWidth), y1=1.0 - 2.0/float(cellHeight);
    return WireFrame(
        {{"position", Attribute{
            2, GL_FLOAT, false,
            0, 0}}},
        {
            x0, y0,  
            x1, y0, 
            x1, y1,
            x0, y1},
        {0, 1, 1, 2, 2, 3, 3, 0},
        WireFrame::LINES);
}

static WireFrame get_container_wire_frame_outer(const SimParams &params) {
    float x0 = 0.0, y0=0.0;
    float x1 = 1.0, y1=1.0;
    return WireFrame(
        {{"position", Attribute{
            2, GL_FLOAT, false,
            0, 0}}},
        {
            x0, y0,  
            x1, y0, 
            x1, y1,
            x0, y1},
        {0, 1, 1, 2, 2, 3, 3, 0},
        WireFrame::LINES);
}

Programs::Programs() {
    this->copy = Quad::make_program_from_path(
        "./shaders/util/copy.frag"
    );
    this->copy_scale = make_program_from_paths(
        "./shaders/util/generic-2d-display.vert",
        "./shaders/util/copy.frag"
    );
    this->particles_view = make_program_from_paths(
        "./shaders/particles-display/circles.vert",
        "./shaders/util/uniform-color.frag"
    );
    this->container_view = make_program_from_paths(
        "./shaders/util/generic-2d-display.vert",
        "./shaders/util/uniform-color.frag"
    ),
    this->uniform_linear_transform = Quad::make_program_from_path(
        "./shaders/util/uniform-linear-transform.frag"
    );
    this->forces = Quad::make_program_from_path(
        "./shaders/dynamics/forces.frag"
    );
    this->energies = Quad::make_program_from_path(
        "./shaders/dynamics/energies.frag"
    );
    this->verlet.positions = Quad::make_program_from_path(
        "./shaders/dynamics/verlet-positions.frag"
    );
    this->verlet.velocities1 = Quad::make_program_from_path(
        "./shaders/dynamics/verlet-velocities1.frag"
    );
    this->verlet.velocities2 = Quad::make_program_from_path(
        "./shaders/dynamics/verlet-velocities2.frag"
    );
}


Frames::
Frames(const TextureParams &default_tex_params, const SimParams &params):
    default_tex_params(default_tex_params),
    position_velocities_tex_params({
        .format=GL_RGBA32F,
        .width=(unsigned int)decompose(params.particleCount)[0],
        .height=(unsigned int)decompose(params.particleCount)[1],
        .generate_mipmap=1, // default_tex_params.generate_mipmap,
        .min_filter=GL_NEAREST,
        .mag_filter=GL_NEAREST,
        .wrap_s=GL_REPEAT,
        .wrap_t=GL_REPEAT
    }),
    forces_tex_params({
        .format=GL_RG32F,
        .width=(unsigned int)decompose(params.particleCount)[0],
        .height=(unsigned int)decompose(params.particleCount)[1],
        .generate_mipmap=1, // default_tex_params.generate_mipmap,
        .min_filter=GL_NEAREST,
        .mag_filter=GL_NEAREST,
        .wrap_s=GL_REPEAT,
        .wrap_t=GL_REPEAT
    }),
    energies_tex_params({
        .format=GL_R32F,
        .width=(unsigned int)decompose(params.particleCount)[0],
        .height=(unsigned int)decompose(params.particleCount)[1],
        .generate_mipmap=1, // default_tex_params.generate_mipmap,
        .min_filter=GL_NEAREST,
        .mag_filter=GL_NEAREST,
        .wrap_s=GL_REPEAT,
        .wrap_t=GL_REPEAT
    }),
    cell_tex_params({
        .format=GL_RG32F,
        .width=(unsigned int)params.cellGridDimensions2D[0],
        .height=(unsigned int)params.cellGridDimensions2D[1],
        .generate_mipmap=1,
        .min_filter=GL_NEAREST,
        .mag_filter=GL_NEAREST,
        .wrap_s=GL_REPEAT,
        .wrap_t=GL_REPEAT
    }),
    cells(cell_tex_params),
    position_velocities {
        Quad(position_velocities_tex_params),
        Quad(position_velocities_tex_params)
    },
    forces {
        Quad(forces_tex_params),
        Quad(forces_tex_params)
    },
    energies(Quad(energies_tex_params)),
    render_tmp(default_tex_params),
    render(default_tex_params),
    quad_wire_frame(get_quad_wire_frame()),
    particles_wire_frame(get_particles_wire_frame(
        decompose(params.particleCount)
    )),
    container_wire_frame_inner(get_container_wire_frame_inner(params)),
    container_wire_frame_outer(get_container_wire_frame_outer(params))
{
}

void Frames::reset_number_of_particles(const SimParams &params) {
    IVec2 tex_dimensions = decompose(params.particleCount);
    this->energies_tex_params.width = tex_dimensions[0];
    this->energies_tex_params.height = tex_dimensions[1];
    this->forces_tex_params.width = tex_dimensions[0];
    this->forces_tex_params.height = tex_dimensions[1];
    this->position_velocities_tex_params.width = tex_dimensions[0];
    this->position_velocities_tex_params.height = tex_dimensions[1];
    for (int i = 0; i < 2; i++) {
        this->position_velocities[i].reset(
            this->position_velocities_tex_params);
        this->forces[i].reset(
            this->forces_tex_params);
    }
    this->energies.reset(energies_tex_params);
    this->particles_wire_frame = get_particles_wire_frame(
        tex_dimensions
    );
}

void Simulation::
init_particles(
    IVec2 particles_per_cell_dimension,
    int left_cell_offset, int bottom_cell_offset,
    int right_cell_offset, int top_cell_offset,
    const SimParams &params) {
    std::random_device rand_device;
    std::default_random_engine rand_engine (rand_device());
    std::uniform_real_distribution<double> rand_mag(0.0, 1.0);
    std::uniform_real_distribution<double> rand_ang(
        0.0, 6.283185307179586);
    std::vector<Vec4> particles{(size_t)params.particleCount};
    IVec2 cellDimensions = params.cellGridDimensions2D;
    Vec2 simDimensions = params.simDimensions2D;
    float cellWidth = simDimensions[0]/cellDimensions[0];
    float cellHeight = simDimensions[1]/cellDimensions[1];
    int i = 0;
    for (int y_ind = bottom_cell_offset; 
         y_ind < cellDimensions[1] - top_cell_offset;
         y_ind++) {
        for (int x_ind = left_cell_offset; 
             x_ind < cellDimensions[0] - right_cell_offset; 
             x_ind++) {
            float x_cell = x_ind*cellWidth;
            float y_cell = y_ind*cellHeight;
            for (int c_y = 0; c_y 
                    < particles_per_cell_dimension[1]; c_y++) {
                for (int c_x = 0; c_x 
                        < particles_per_cell_dimension[0]; c_x++) {
                    if (i < params.particleCount) {
                        particles[i].x = x_cell 
                            + (c_x + 0.5)
                            * (cellWidth/particles_per_cell_dimension[0]);
                        particles[i].y = y_cell 
                            + (c_y + 0.5)
                            * (cellHeight/particles_per_cell_dimension[1]);
                        float v_mag
                             = rand_mag(rand_engine)*params.sigma*0.005;
                        float v_ang = rand_ang(rand_engine);
                        particles[i].z = v_mag*cos(v_ang);
                        particles[i].w = v_mag*sin(v_ang);
                    }
                    i++;
                }
            }
        }
    }
    m_frames.position_velocities[0].set_pixels((float *)&particles[0][0]);
    m_sort(m_frames.position_velocities[0], m_frames.position_velocities[0]);
    ConstructCellsGridRes construct_cells_grid_res = construct_cells_grid(
        m_frames.cells, m_frames.position_velocities[0], params);
    int max_particle_per_cell_count = construct_cells_grid_res.max_cell_count;
    m_max_velocity = construct_cells_grid_res.max_velocity;
    compute_forces(m_frames.forces[0],
        m_frames.position_velocities[0], m_frames.cells, 
        max_particle_per_cell_count, params);
    compute_energies(
        m_frames.energies, m_frames.position_velocities[0],
        m_frames.cells, max_particle_per_cell_count, params);
    m_energy_start = sum_energies(m_frames.energies, params);
    m_energy = m_energy_start;
    Vec2 max_force = this->find_max_force(m_frames.forces[0]);
    printf("Max force: %g, %g\n", max_force.x, max_force.y);
    printf(
        "Max number of particles per cell: %d\n",
        max_particle_per_cell_count);
    m_dt = params.requestedDt;
}

void Simulation::
init_box_particles(
    Vec2 position, Vec2 velocity, 
    float spacing, float angular_offset,
    const SimParams &params) {
    std::vector<Vec4> particles{(size_t)params.particleCount};
    IVec2 dimensions = decompose(params.particleCount);
    int ind = 0;
    for (int y_ind = 0; y_ind < dimensions[1]; y_ind++) {
        for (int x_ind = 0; x_ind < dimensions[0]; x_ind++) {
            float x_pos0 = spacing*float(x_ind);
            float y_pos0 = spacing*float(y_ind);
            float x_pos 
                = position.x 
                + x_pos0*cos(angular_offset) - y_pos0*sin(angular_offset);
            float y_pos 
                = position.y
                + x_pos0*sin(angular_offset) + y_pos0*cos(angular_offset);
            particles[ind] = Vec4{.ind{
                x_pos, y_pos, velocity.x, velocity.y
            }};
            ind++;
        }
    }
    m_frames.position_velocities[0].set_pixels((float *)&particles[0][0]);
    m_sort(m_frames.position_velocities[0], m_frames.position_velocities[0]);
    ConstructCellsGridRes construct_cells_grid_res = construct_cells_grid(
        m_frames.cells, m_frames.position_velocities[0], params);
    int max_particle_per_cell_count = construct_cells_grid_res.max_cell_count;
    m_max_velocity = construct_cells_grid_res.max_velocity;
    compute_forces(m_frames.forces[0],
        m_frames.position_velocities[0], m_frames.cells, 
        max_particle_per_cell_count, params);
    compute_energies(
        m_frames.energies, m_frames.position_velocities[0],
        m_frames.cells, max_particle_per_cell_count, params);
    m_energy_start = sum_energies(m_frames.energies, params);
    m_energy = m_energy_start;
    Vec2 max_force = this->find_max_force(m_frames.forces[0]);
    printf("Max force: %g, %g\n", max_force.x, max_force.y);
    printf(
        "Max number of particles per cell: %d\n",
        max_particle_per_cell_count);
    m_dt = params.requestedDt;
}

Simulation::
Simulation(const TextureParams &default_tex_params, const SimParams &params
) : m_programs(), m_frames(default_tex_params, params), 
    m_sort(
        decompose(params.particleCount), 
        sort::COMPARE_2D_X_MAJOR_CELL_IND,
        sort::ComparisonParams{
            .cells{
                .dimensions=Vec3{.ind{
                    params.simDimensions2D[0] /
                    float(params.cellGridDimensions2D[0]),
                    params.simDimensions2D[1] /
                    float(params.cellGridDimensions2D[1]),
                    0.0}},
                .grid_dimensions=IVec3{.ind{
                    params.cellGridDimensions2D[0],
                    params.cellGridDimensions2D[1],
                    0}},
                .grid_origin=Vec3{.x=0.0, .y=0.0, .z=0.0}
            }
        }
    ) {
    m_forces[0] = &m_frames.forces[0];
    m_forces[1] = &m_frames.forces[1];
    m_position_velocities[0] = &m_frames.position_velocities[0];
    m_position_velocities[1] = &m_frames.position_velocities[1];
    m_positions_velocities_vec = std::vector<Vec4>{(size_t)params.particleCount};
    m_forces_vec = std::vector<Vec2>{(size_t)params.particleCount};
    m_energies_vec = std::vector<float>((size_t)params.particleCount, 0.0);
    m_energy = 0.0;
    m_cell_array = std::vector<Vec2>{
        (size_t)params.cellGridDimensions2D[0]*params.cellGridDimensions2D[1]};
    init_box_particles({
        .x=params.simDimensions2D[0]*0.5F, .y=params.simDimensions2D[1]*0.075F},
        {.x=0.0, .y=0.0},
        1.1*params.sigma, 
        0.75, params);
    
}

ConstructCellsGridRes Simulation::construct_cells_grid(
    Quad &cells, Quad &positions,
    const SimParams &params) {
    positions.fill_array_with_contents(
        (float *)&m_positions_velocities_vec[0]);
    int cell_ind = 0;
    for (int i = 0; 
         i < params.cellGridDimensions2D[0]*params.cellGridDimensions2D[1]; i++)
        m_cell_array[i] = {.x=0.0, .y=0.0};
    int max_particle_per_cell_count = 0;
    Vec2 max_velocity {.x=0.0, .y=0.0};
    float max_vel_mag2 = 0.0;
    for (int i = 0; i < params.particleCount; i++) {
        Vec4 *positions_arr = (Vec4 *)&m_positions_velocities_vec[0];
        Vec2 vel = {
            .x=m_positions_velocities_vec[i][0],
            .y=m_positions_velocities_vec[i][1]};
        double tmp;
        if ((tmp = ((double)vel.x*(double)vel.x 
                + (double)vel.y*(double)vel.y)) > max_vel_mag2) {
            max_velocity = vel;
            max_vel_mag2 = tmp;
        }
        float x = positions_arr[i].x, y = positions_arr[i].y;
        float cell_width = params.simDimensions2D[0]
            / float(params.cellGridDimensions2D[0]);
        float cell_height = params.simDimensions2D[1]
            / float(params.cellGridDimensions2D[1]);
        float cell_x_ind = floor(x/cell_width);
        float cell_y_ind = floor(y/cell_height);
        int cell_ind_next 
            = int(cell_x_ind 
                    + cell_y_ind*float(params.cellGridDimensions2D[0]));
        m_cell_array[cell_ind_next][1] += 1.0;
        if (i == 0) {
            cell_ind = cell_ind_next;
            m_cell_array[cell_ind][0] = i;
        }
        if (cell_ind_next != cell_ind) {
            m_cell_array[cell_ind_next][0] = i;
            if (m_cell_array[cell_ind][1] > max_particle_per_cell_count)
                max_particle_per_cell_count = m_cell_array[cell_ind][1];
            cell_ind = cell_ind_next;
        }
        if (i == params.particleCount - 1) {
            if (m_cell_array[cell_ind_next][1] > max_particle_per_cell_count)
                max_particle_per_cell_count = m_cell_array[cell_ind_next][1];
        }
    }
    cells.set_pixels((float *)&m_cell_array[0]);
    return {
        .max_cell_count=max_particle_per_cell_count,
        .max_velocity=max_velocity
    };
}

void Simulation::compute_forces(
    Quad &forces, const Quad &positions, const Quad &cells,
    int max_particle_count_per_cell, 
    const SimParams &params) {
    Vec2 cell_dimensions = {.ind{
        params.simDimensions2D[0]/float(params.cellGridDimensions2D[0]),
        params.simDimensions2D[1]/float(params.cellGridDimensions2D[1])

    }};
    IVec2 particles_dimensions2d = decompose(params.particleCount);
    forces.draw(
        m_programs.forces,
        {
            {"sigma", params.sigma},
            {"epsilon", params.epsilon},
            {"gForce", params.gForce},
            {"wallForce", params.wallForce},
            {"simDimensions2D", params.simDimensions2D},
            {"cellsTex", &cells},
            {"gridCellsDimensions2D", params.cellGridDimensions2D},
            {"cellDimensions2D", cell_dimensions},
            {"positionVelocitiesTex", &positions},
            {"particlesTexDimensions2D", particles_dimensions2d},
            {"maxParticlesPerCellCount", int(max_particle_count_per_cell)},
            {"neighbourCellCountForForceComputation",
                int(params.neighbourCellCountForForceComputation)}
        }
    );
}

Vec2 Simulation::find_max_force(const Quad &forces) {
    forces.fill_array_with_contents((float *)&m_forces_vec[0]);
    float mag2 = 0.0;
    Vec2 max_force {};
    for (auto &f: m_forces_vec) {
        if (f.length_squared() > mag2) {
            max_force = f;
            mag2 = f.length_squared();
        }
    }
    return max_force;
}

void Simulation::compute_energies(
    Quad &energies, const Quad &position_velocities, const Quad &cells,
    int max_particle_count_per_cell, 
    const SimParams &params) {
    Vec2 cell_dimensions = {.ind{
        params.simDimensions2D[0]/float(params.cellGridDimensions2D[0]),
        params.simDimensions2D[1]/float(params.cellGridDimensions2D[1])

    }};
    IVec2 particles_dimensions2d = decompose(params.particleCount);
    energies.draw(
        m_programs.energies,
        {
            {"sigma", params.sigma},
            {"epsilon", params.epsilon},
            {"gForce", params.gForce},
            {"wallForce", params.wallForce},
            {"simDimensions2D", params.simDimensions2D},
            {"cellsTex", &cells},
            {"gridCellsDimensions2D", params.cellGridDimensions2D},
            {"cellDimensions2D", cell_dimensions},
            {"positionVelocitiesTex", &position_velocities},
            {"particlesTexDimensions2D", particles_dimensions2d},
            {"maxParticlesPerCellCount", int(max_particle_count_per_cell)},
            {"neighbourCellCountForForceComputation",
                int(params.neighbourCellCountForForceComputation)}

        }
    );
}

float Simulation::sum_energies(
    const Quad &energies, const SimParams &params) {
    energies.fill_array_with_contents(&m_energies_vec[0]);
    m_energy = 0.0;
    for (int i = 0; i < params.particleCount; i++)
        m_energy += m_energies_vec[i];
    return m_energy;
}

float Simulation::get_energy() {
    return m_energy;
}

float Simulation::get_energy_drift_error() {
    return m_energy - m_energy_start;
}

float Simulation::get_energy_drift_error_percentage() {
    return 100.0*get_energy_drift_error()/std::abs(m_energy_start);
}

double get_smallest_positive_root(
    double a, double b, double c) {
    double x1 = (-b - sqrt(b*b - 4.0*a*c))/(2.0*a);
    double x2 = (-b + sqrt(b*b - 4.0*a*c))/(2.0*a);
    if (x1 >= 0.0 && x2 >= 0.0) {
        return std::min(x1, x2);
    } else {
        if (x1 > 0.0)
            return x1;
        else
            return x2;
    }

}

float Simulation::time_step(const SimParams &params) {
    Vec2 max_force = this->find_max_force(m_frames.forces[0]);
    m_dt = std::min(
        (float)get_smallest_positive_root(
            0.5*max_force.length(), 
            m_max_velocity.length(), 
            -params.sigma/params.limitTimeStepAgressiveness),
        params.requestedDt);
    // printf("Desired dt: %g\n", m_dt);
    // printf("Time Step %d\n", params.nSteps);
    // printf("Advancing positions.\n");
    m_position_velocities[1]->draw(
        m_programs.verlet.positions,
        {
            {"dt", m_dt},
            {"positionVelocitiesTex", m_position_velocities[0]},
            {"enforcePeriodicity", int(true)},
            {"simDimensions2D", params.simDimensions2D},
            {"forcesTex", m_forces[0]}
        }
    );
    // printf("Advancing velocities using forces at initial positions.\n");
    m_position_velocities[0]->draw(
        m_programs.verlet.velocities1,
        {
            {"dt", m_dt},
            {"positionVelocitiesTex", m_position_velocities[1]},
            {"forcesTex", m_forces[0]}
        }
    );
    // printf("Sorting positions.\n");
    // if (params.nSteps % 10) 
    {
        m_sort(*m_position_velocities[0], *m_position_velocities[0]);
        ConstructCellsGridRes res = this->construct_cells_grid(
            m_frames.cells,
            *m_position_velocities[0], params);
        m_counts_per_cell = res.max_cell_count;
        m_max_velocity = res.max_velocity;
    }
    // printf("Computing forces from updated positions.\n");
    this->compute_forces(
        *m_forces[1],
        *m_position_velocities[0], m_frames.cells,
        m_counts_per_cell, params);
    // printf("Max cell count: %d\n", count_per_cell);
    // printf("Fully updating velocities.\n");
    m_position_velocities[1]->draw(
        m_programs.verlet.velocities2,
        {
            {"dt", m_dt},
            {"positionVelocitiesTex", m_position_velocities[0]},
            {"forcesTex", m_forces[1]}
        }
    );
    compute_energies(
        m_frames.energies, *m_position_velocities[1],
        m_frames.cells, m_counts_per_cell, params);
    sum_energies(m_frames.energies, params);
    std::swap(m_forces[0], m_forces[1]);
    std::swap(m_position_velocities[0], m_position_velocities[1]);
    // printf("Max force mag.: %g\n", max_force.length());
    // printf("max delta r: %g\n", max_velocity.length()*params.requestedDt + 0.5*max_force.length()*params.requestedDt*params.requestedDt);
    // 0 = r + v*dt + 0.5*f*dt**2
    return m_dt;
}

const RenderTarget &Simulation::view(SimParams &params, float scale, Vec2 translate) {
    m_frames.render.clear();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float b = params.particleBrightness;
    m_frames.render.draw(
        m_programs.container_view,
        {
            {"scale", scale},
            {"translate", translate},
            {"color", Vec4{.ind{1.0, 1.0, 1.0, 1.0}}}
        },
        m_frames.container_wire_frame_inner
    );
    m_frames.render.draw(
        m_programs.container_view,
        {
            {"scale", scale},
            {"translate", translate},
            {"color", Vec4{.ind{1.0, 1.0, 1.0, 1.0}}}
        },
        m_frames.container_wire_frame_outer
    );
    m_frames.render.draw(
        m_programs.particles_view,
        {
            {"coordTex", m_position_velocities[0]},
            {"circleRadius", 0.5F*params.sigma/params.simDimensions2D[0]},
            {"dimensions2D", params.simDimensions2D},
            {"color", Vec4{.ind{b, b, b, 1.0}}},
            {"scale", scale},
            {"translate", translate}
        },
        m_frames.particles_wire_frame
    );
    // m_frames.render.draw(
    //     m_programs.uniform_linear_transform,
    //     {
    //         {"tex", m_forces[0]},
    //         {"m", Vec4{.r=1.0, .g=1.0, .b=1.0, .a=1.0}},
    //         {"b", Vec4{.r=0.0, .b=0.0, .g=0.0, .a=0.0}}
    //     },
    //     m_frames.quad_wire_frame
    // );
    if (params.showCellsByParticleCount) {
        m_frames.render_tmp.clear();
        m_frames.render_tmp.draw(
            m_programs.uniform_linear_transform,
            {
                {"tex", &m_frames.cells},
                {"row0", Vec4{.ind{0.0, 1.0, 0.0, 0.0}}},
                {"row1", Vec4{.ind{0.0, 1.0, 0.0, 0.0}}},
                {"row2", Vec4{.ind{0.0, 1.0, 0.0, 0.0}}},
                {"row3", Vec4{.ind{0.0, 0.0, 0.0, 0.0}}},
                {"b", Vec4{.r=0.0, .g=0.0, .b=0.0, .a=0.25}}
            },
            m_frames.quad_wire_frame
        );
        m_frames.render.draw(
            m_programs.copy_scale,
            {
                {"scale", scale},
                {"translate", translate},
                {"tex", &m_frames.render_tmp}
            },
            m_frames.quad_wire_frame
        );
    }
    glDisable(GL_BLEND);
    return m_frames.render;
}

void Simulation::reset_number_of_particles(const SimParams &params) {
    m_positions_velocities_vec.resize(params.particleCount);
    m_forces_vec.resize(params.particleCount);
    m_energies_vec.resize(params.particleCount);
    m_frames.reset_number_of_particles(params);
    IVec2 tex_dimensions2d = {
        .x=(int)m_frames.position_velocities_tex_params.width,
        .y=(int)m_frames.position_velocities_tex_params.height};
    m_sort.reset_dimensions(tex_dimensions2d);
    m_forces[0] = &m_frames.forces[0];
    m_forces[1] = &m_frames.forces[1];
    m_position_velocities[0] = &m_frames.position_velocities[0];
    m_position_velocities[1] = &m_frames.position_velocities[1];
    if (params.particleCount > 17000) {
        init_particles({.ind{4, 4}}, 3, 3, 3, 3, params);
        return;
    }
    init_box_particles({
        .x=params.simDimensions2D[0]*0.5F, .y=params.simDimensions2D[1]*0.075F},
        {.x=0.0, .y=0.0},
        1.1*params.sigma, 
        0.75, params);
}

float Simulation::get_actual_time_step() const {
    return m_dt;
}