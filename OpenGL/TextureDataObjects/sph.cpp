#include "sph.hpp"

// #include <OpenGL/OpenGL.h>
#include <cstdlib>
#include <new>
#define GL_SILENCE_DEPRECATION

#include "texture_data.hpp"
#include "draw_texture_data.hpp"
// #include <OpenGL/OpenGL.h>
#include <vector>
#include "fft.h"
#include <iostream>
#include <ctime>
#include "bitmap.h"
#include "summation.h"
#include <GLES3/gl3.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <functional>

static std::function <void()> loop;
#ifdef __EMSCRIPTEN__
static void main_loop() {
    loop();
}
#endif

static const int N_PARTICLES = 2048;

#define PI 3.141592653589793

static inline float max(float a, float b) {
    return (a > b)? a: b;
}

static inline float min(float a, float b) {
    return (a < b)? a: b;
}

/* This struct is passed as the last argument for pthread_create.
 It is the data used for a single thread. */
struct ThreadData {
    int start; // Start index of the particles for which this thread
               // will iterate over
    int end;  // End index
    int n_particles; // The total number of particles
    float k; // Specific gas constant used for computing pressure
    float rest_density; // density at which the pressure should be zero
    float nu; // Constant that controls the viscosity of the fluid
    const float *masses; // mass for each particle
    const struct Vec2 *positions; // particles' positions
    const struct Vec2 *velocities; // particles' velocities
    const float *densities; // Densities assigned to each particle
    float *mut_densities; // Array to modify the densities
    struct Vec2 *forces; // Force for each particle
    // float *energies;
};

#define MAX_SUPPORTED_THREADS 32
static int number_of_threads_to_use() {
    // Number of threads to use
    // https://stackoverflow.com/a/150971
#ifndef WIN_32
    int n_threads = sysconf(_SC_NPROCESSORS_ONLN);
#else
    int n_threads = 4
#endif

#ifdef __EMSCRIPTEN__
    n_threads = 8;
#endif

        if (n_threads > MAX_SUPPORTED_THREADS)
            return MAX_SUPPORTED_THREADS;
    return n_threads;

}

static struct Vec2
get_ext_wall_force(struct Vec2 r, float wall,
                   float lw, float rw, float uw, float dw) {
    struct Vec2 left_wall_force
         {.x=(r.x < lw)? -wall*(r.x - lw): 0.0F, .y=0.0F};
    struct Vec2 right_wall_force
        {.x=(r.x > rw)? -wall*(r.x - rw): 0.0F, .y=0.0F};
    struct Vec2 ceil_force
        {.x=0.0F, .y=((r.y > uw)? -wall*(r.y - uw): 0.0F)};
    struct Vec2 floor_force
        {.x=0.0F, .y=((r.y < dw)? -wall*(r.y - dw): 0.0F)};
    struct Vec2 ext_wall_force {.x=0.0F, .y=0.0F};
    for (int i = 0; i < 2; i++)
        ext_wall_force.ind[i]
            = left_wall_force.ind[i] + right_wall_force.ind[i]
              + ceil_force.ind[i] + floor_force.ind[i];
    return ext_wall_force;
}

struct {
    float sigma_x;
    float sigma_y;
} gaussian_params = {.sigma_x=0.005, .sigma_y=0.005};



static inline bool in_gaussian_interaction_distance(float r2) {
    float sx = gaussian_params.sigma_x;
    float sy = gaussian_params.sigma_y;
    return r2 < ((4.0*sx)*(4.0*sx) + (4.0*sy)*(4.0*sy));
}


static inline double
gaussian_weight_function(float val, struct Vec2 r) {
    float sx = gaussian_params.sigma_x;
    float sy = gaussian_params.sigma_y;
    return val*exp(-0.5*(r.x*r.x/(sx*sx) + r.y*r.y/(sy*sy)))/(2.0*PI*sx*sy);
}

static inline Vec2
gaussian_grad_weight_function(float val, struct Vec2 r) {
    float sx = gaussian_params.sigma_x;
    float sy = gaussian_params.sigma_y;
    float x = val*exp(-0.5*(r.x*r.x/(sx*sx) + r.y*r.y/(sy*sy)))/(2.0*PI*sx*sy)
        *(-r.x/(sx*sx));
    float y = val*exp(-0.5*(r.x*r.x/(sx*sx) + r.y*r.y/(sy*sy)))/(2.0*PI*sx*sy)
        *(-r.y/(sy*sy));
    return {.x=x, .y=y};
}

static inline double
gaussian_laplacian_weight_function(float val, struct Vec2 r) {
    float sx = gaussian_params.sigma_x;
    float sy = gaussian_params.sigma_y;
    float g = gaussian_weight_function(val, r);
    float g_xx = g*(r.x*r.x/(sx*sx*sx*sx) - 1.0/(sx*sx));
    float g_yy = g*(r.y*r.y/(sy*sy*sy*sy) - 1.0/(sy*sy));
    return g_xx + g_yy;
}


static inline bool in_interaction_distance(float r2) {
    return in_gaussian_interaction_distance(r2);
}

static inline double
weight_function(float val, struct Vec2 r) {
    return gaussian_weight_function(val, r);
}

static inline Vec2
grad_weight_function(float val, struct Vec2 r) {
    return gaussian_grad_weight_function(val, r);
}

static inline double
laplacian_weight_function(float val, struct Vec2 r) {
    return gaussian_laplacian_weight_function(val, r);
}

static inline float
compute_pressure(float k, float rest_density, float density) {
    // return k*(density - rest_density);
    return max(k*(density - rest_density), 0.0);
}

/* The densities for each particle need to be computed first
 before using these to compute the remaining properties.
 */
static void *threaded_compute_densities(void *void_data) {
    struct ThreadData *data = (struct ThreadData *)void_data;
    int n_particles = data->n_particles;
    int iter_start = data->start;
    int iter_end = data->end;
    const struct Vec2 *positions = data->positions;
    const float *masses = data->masses;
    float *mut_densities = data->mut_densities;
    for (int j = iter_start; j < iter_end; j++) {
        float rj_x = positions[j].x;
        float rj_y = positions[j].y;
        float mass_j = masses[j];
        for (int i = j; i < n_particles; i++) {
            float ri_x = positions[i].x;
            float ri_y = positions[i].y;
            float mass_i = masses[i];
            float rx = ri_x - rj_x;
            float ry = ri_y - rj_y;
            float r2 = rx*rx + ry*ry;
            struct Vec2 r_ij {.x=rx, .y=ry};
            if (in_interaction_distance(r2)) {
                mut_densities[i]
                    += weight_function(mass_j, r_ij);
                mut_densities[j]
                    += weight_function(mass_i, r_ij);
            }
        }
    }
    return NULL;
}

static void *threaded_compute_energies_and_forces(void *void_data) {
    struct ThreadData *data = (struct ThreadData *)void_data;
    int n_particles = data->n_particles;
    float rest_density = data->rest_density;
    float k = data->k;
    float nu = data->nu;
    // const struct Vec2 *positions = data->positions;
    const float *densities = data->densities;
    const float *masses = data->masses;
    int iter_start = data->start;
    int iter_end = data->end;
    for (int j = iter_start; j < iter_end; j++) {
        float rj_x = data->positions[j].x;
        float rj_y = data->positions[j].y;
        float vj_x = data->velocities[j].x;
        float vj_y = data->velocities[j].y;
        float density_j = densities[j];
        float mass_j = masses[j];
        for (int i = j; i < n_particles; i++) {
            float ri_x = data->positions[i].x;
            float ri_y = data->positions[i].y;
            float vi_x = data->velocities[i].x;
            float vi_y = data->velocities[i].y;
            float density_i = densities[i];
            float mass_i = masses[i];
            float rx = ri_x - rj_x;
            float ry = ri_y - rj_y;
            float r2 = rx*rx + ry*ry;
            struct Vec2 r {.x=rx, .y=ry};
            if (in_interaction_distance(r2)) {
                struct Vec2 grad_w = grad_weight_function(1.0, r);
                // Compute the pressure
                // (this is the average of the pressure at i and j so that
                //  pressure_ij is symmetric and grad_w_ij is antisymmetric,
                //  ensuring that Newton's third law is obeyed)
                float pressure
                    = 0.5*(compute_pressure(k, rest_density,
                                            density_i)
                           + compute_pressure(k, rest_density,
                                              density_j));
                data->forces[j].x += mass_i*pressure*grad_w.x/density_i;
                data->forces[j].y += mass_i*pressure*grad_w.y/density_i;
                data->forces[i].x -= mass_j*pressure*grad_w.x/density_j;
                data->forces[i].y -= mass_j*pressure*grad_w.y/density_j;
                
                float del2_w = laplacian_weight_function(1.0, r);
                float visc_x = nu*(vi_x - vj_x)*del2_w;
                float visc_y = nu*(vi_y - vj_y)*del2_w;
                data->forces[j].x += mass_i/density_i*visc_x;
                data->forces[j].y += mass_i/density_i*visc_y;
                data->forces[i].x -= mass_j/density_j*visc_x;
                data->forces[i].y -= mass_j/density_j*visc_y;
            }
        }
    }
    return NULL;
}

static double time_difference_in_ms(const struct timespec *t1,
                                    const struct timespec *t2);

/* SPH Simulation with some issues that still need to be resolved
 * (the simulation may give different results even with the same
 *  initial conditions, and sometimes very high velocity particles
 *  may appear where they easily cut through the bulk of the fluid).
 * This initial implementation was made by following these lecture notes:
 * https://www.cs.cmu.edu/~scoros/cs15467-s16/lectures/11-fluids2.pdf.
 *
 * References:
 *
 * - Coros S. Lecture 10: Particle-based Fluids.
 *   Simulation Methods for Animation and Digital Fabrication.
 *   https://www.cs.cmu.edu/~scoros/cs15467-s16/lectures/11-fluids2.pdf
 *
 * - Wikipedia contributors. (2022, November 8).
 *   Smoothed-particle hydrodynamics. In Wikipedia, The Free Encyclopedia.
 *   https://en.wikipedia.org/wiki/Smoothed-particle_hydrodynamics
 * 
 * Multithreading reference:
 * 
 * - Ippolito, G. (2004). Posix thread (pthread) libraries.
 *   YoLinux Tutorials.
 *   https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/\
 *   www/pthreads.html.
 */
int sph_mt(GLFWwindow *window, frame_id main_frame) {
    int exit_status = 0;
    int window_width {}, window_height {};
    window_dimensions(window, &window_width, &window_height);
    frame_id view_program = make_quad_program("./shaders/copy.frag");

    // Variables used by the simulation directly
    double t = 0.0; // Simulation time elapsed
    double velocity_max = 0.0; // Used for setting the time step
    double dt = 0.00000001; // time step
    float rest_density = 10000.0;
    float specific_gas_constant = 1e7;
    double wall_spring = 5.0e15; // Force on the particle imparted by the wall
    double gravity_force = -10000000.0;
    double viscosity_constant = 5e4;
    std::vector<float> masses {};
    // Density arrays used in the time step function, 
    // where these arrays must be swapped after ach step.
    std::vector<float> densities0 {};
    std::vector<float> densities1 {};
    // Force arrays used for the time step function. These need to be
    // swapped after each time step.
    std::vector<struct Vec2> force0 {N_PARTICLES};
    std::vector<struct Vec2> force1 {N_PARTICLES};

    // Initialize positions and masses of the particles.
    auto p0_arr = std::vector<struct Vec2> {};
    auto v0_arr = std::vector<struct Vec2> {};
    auto p1_arr = std::vector<struct Vec2> {};
    auto v1_arr = std::vector<struct Vec2> {};
    {
        for (int i = 0; i < N_PARTICLES; i++) {
            masses.push_back(1.0);
            densities0.push_back(0.0);
            densities1.push_back(0.0);
            // float rx = (float)(i % 64)/(float)128 + 0.25;
            // float ry = (float)(i / 64)/(float)128 + 0.25;
            float offset = ((i / 70) % 2 == 1)? 0.5/120: 0.0;
            // float offset = 0.0;
            float rx = (float)(i % 70)/(float)120 + offset + 0.15;
            float ry = (float)(i / 70)/(float)120 + 0.07;
            // float rx = (float)(i % 128)/(float)128 + 0.25;
            // float ry = (float)(i / 128)/(float)128 + 0.25;
            double theta = -0.25;
            float rx2 = rx; // cos(theta)*rx - sin(theta)*ry;
            float ry2 = ry; // sin(theta)*rx + cos(theta)*ry;
            p0_arr.push_back({.x = rx2, .y = ry2});
            v0_arr.push_back({.x = 0.0F*1120.0F*ry2, .y = 0.0F*11200.0F*rx2});
            p1_arr.push_back({.x = rx2, .y = ry2});
            v1_arr.push_back({.x = 0.0F*1120.0F*ry2, .y = 0.0F*11200.0F*rx2});
        }
    }

    // These are used for viewing and are not used for actual computations
    // in the simulation.
    auto r = Texture2DData((struct Vec2 *)&p0_arr[0], N_PARTICLES, 1);
    auto v = Texture2DData((struct Vec2 *)&v0_arr[0], N_PARTICLES, 1);

    const int N_THREADS = number_of_threads_to_use();

    // Number of particles to iterate over per each thread
    // int updates_per_th = N_PARTICLES/N_THREADS;

    // The particles are subdivided into separate groups where each
    // group is handled by its own thread.
    // Each group is given its own array that stores the cumulative
    // inter-particle forces for only the particles in their group.
    // The threads for each group are executed and parallel,
    // and after the threads merge the arrays from each group
    // are then summed to get the total inter-particle force
    // on each particle.
    std::vector<std::vector<struct Vec2>> force_th {};
    std::vector<std::vector<float>> density_th {};
    for (int i = 0; i < N_THREADS; i++) {
        force_th.push_back(std::vector<struct Vec2> {N_PARTICLES});
        density_th.push_back(std::vector<float> {N_PARTICLES});
    }

    // This keeps track of which particle index range each thread
    // will work on.
    // Ideally this is subdivided in a way such that each thread does
    // the same number of operations.
    std::vector<int> thread_starts {};
    std::vector<int> thread_ends {};
    {
        auto quad_root_pos = [](double a, double b, double c) {
            return (-b + sqrt(b*b - 4.0*a*c))/(2.0*a);
        };
        double total_ops = N_PARTICLES*(N_PARTICLES/2.0 + 0.5);
        thread_starts.push_back(0);
        for (int i = N_THREADS - 1; i >= 1; i--) {
            double root
                = quad_root_pos(0.5, 0.5,
                                -((double)i)*total_ops/((double)N_THREADS));
            thread_starts.push_back(N_PARTICLES - int(root));
            thread_ends.push_back(N_PARTICLES - int(root));

        }
        thread_ends.push_back(N_PARTICLES);
        for (auto &e: thread_ends) std::cout << e << std::endl;
    }

    auto step = [&] (struct Vec2 *r1, struct Vec2 *v1,
                     const struct Vec2 *r0, const struct Vec2 *v0,
                     double t, double dt) {

        auto threaded_densities_update = [&](float *densities,
                                             const float *masses,
                                             const struct Vec2 *positions) {
            int n_th = 0;
            pthread_t threads[MAX_SUPPORTED_THREADS] = {0,};
            struct ThreadData thread_data_arr[MAX_SUPPORTED_THREADS] = {};
            for (; n_th < N_THREADS; n_th++) {
                thread_data_arr[n_th].start = thread_starts[n_th];
                thread_data_arr[n_th].end = thread_ends[n_th];
                thread_data_arr[n_th].n_particles = N_PARTICLES;
                thread_data_arr[n_th].k = specific_gas_constant;
                thread_data_arr[n_th].rest_density = rest_density;
                thread_data_arr[n_th].positions = positions;
                thread_data_arr[n_th].mut_densities = densities;
                thread_data_arr[n_th].masses = masses;
                pthread_create(&threads[n_th], NULL,
                               threaded_compute_densities,
                               (void *)&thread_data_arr[n_th]);
            }
            for (int i = 0; i < N_THREADS; i++) {
                pthread_join(threads[i], NULL);
                for (int j = 0; j < N_THREADS; j++) {
                    densities[i] += density_th[i][j];
                }
            }
        };

        auto threaded_force_update = [&](struct Vec2 *forces,
                                         const float *masses,
                                         const float *densities,
                                         const struct Vec2 *velocities,
                                         const struct Vec2 *positions) {
            int n_th = 0;
            pthread_t threads[MAX_SUPPORTED_THREADS] = {0,};
            struct ThreadData thread_data_arr[MAX_SUPPORTED_THREADS] = {};
            for (; n_th < N_THREADS; n_th++) {
                thread_data_arr[n_th].start = thread_starts[n_th];
                thread_data_arr[n_th].end = thread_ends[n_th];
                thread_data_arr[n_th].n_particles = N_PARTICLES;
                thread_data_arr[n_th].k = specific_gas_constant;
                thread_data_arr[n_th].nu = viscosity_constant;
                thread_data_arr[n_th].rest_density = rest_density;
                thread_data_arr[n_th].positions = positions;
                thread_data_arr[n_th].forces = forces;
                thread_data_arr[n_th].velocities = velocities;
                thread_data_arr[n_th].densities = densities;
                thread_data_arr[n_th].masses = masses;
                pthread_create(&threads[n_th], NULL,
                               threaded_compute_energies_and_forces,
                               (void *)&thread_data_arr[n_th]);
            }
            for (int i = 0; i < N_THREADS; i++) {
                pthread_join(threads[i], NULL);
                for (int j = 0; j < N_PARTICLES; j++) {
                    forces[j].x += force_th[i][j].x;
                    forces[j].y += force_th[i][j].y;
                    if (i == N_THREADS - 1) {
                        struct Vec2 wall_force
                            = get_ext_wall_force({positions[j]},
                                                 wall_spring,
                                                 0.01, 0.99,
                                                 0.99, 0.01);
                        forces[j].x
                            += wall_force.x;
                        forces[j].y
                            += gravity_force*densities[j] + wall_force.y;
                    }
                }
            }
        };

        // Set the force array that will be used for updating the positions
        // of the particles to zero.
        for (int i = 0; i < N_PARTICLES; i++) {
            force1[i].x = 0.0;
            force1[i].y = 0.0;
            densities1[i] = 0.0;
            for (int k = 0; k < force_th.size(); k++) {
                force_th[k][i].x = 0.0;
                force_th[k][i].y = 0.0;
            }
        }

        // If this is the first step, compute the initial densities and forces
        if (t < dt) {
            threaded_densities_update(&densities0[0], &masses[0], r0);
            threaded_force_update(&force0[0], &masses[0], &densities0[0],
                                  v0, r0);
        }

        // temporary velocities array
        std::vector<struct Vec2> v_temp{N_PARTICLES};

        // Compute the updated positions.
        for (int i = 0; i < N_PARTICLES; i++) {
            float density0_i {densities0[i]};
            struct Vec2 force0_i {force0[i]};
            r1[i].x = r0[i].x + v0[i].x*dt + 0.5*dt*dt*force0_i.x/density0_i;
            r1[i].y = r0[i].y + v0[i].y*dt + 0.5*dt*dt*force0_i.y/density0_i;
            v_temp[i].x = v0[i].x + dt*force0_i.x/density0_i;
            v_temp[i].y = v0[i].y + dt*force0_i.y/density0_i;
        }

        // Use the updated positions to compute the updated densities
        threaded_densities_update(&densities1[0], &masses[0], r1);
        // Compute the updated forces.
        threaded_force_update(&force1[0], &masses[0], &densities1[0],
                              &v_temp[0], r1);

        // Compute the updated velocities using the previous and updated
        // forces and densities.
        double v2_max = 0.0;
        for (int i = 0; i < N_PARTICLES; i++) {
            float density0_i {densities0[i]};
            float density1_i {densities1[i]};
            v1[i].x = v0[i].x + 0.5*dt*(force0[i].x/density0_i
                                        + force1[i].x/density1_i);
            v1[i].y = v0[i].y + 0.5*dt*(force0[i].y/density0_i
                                        + force1[i].y/density1_i);
            float v2 = v1[i].x*v1[i].x + v1[i].y*v1[i].y;
            if (v2_max < v2)
                v2_max = v2;
        }
        velocity_max = sqrt(v2_max);
    };

    // Particles view
    int particles_program = make_program(
                                         "./shaders/particle-vert.vert",
                                         "./shaders/colour.frag");
    std::string uvIndexStr ("uvIndex");
    struct VertexParam vertex_params[1] = {{
            .name=&uvIndexStr[0], .size=2, .type=GL_FLOAT,
            .normalized=GL_FALSE, .stride=2*sizeof(float),
            .offset=0,
        },};
    int particles_frame;
    auto vertices_vec
        = std::vector<struct Vec2>((size_t)N_PARTICLES);
    for (int i = 0; i < N_PARTICLES; i++) {
        vertices_vec[i].x = ((float)i + 0.5)/(float)N_PARTICLES;
        vertices_vec[i].y = 0.5;
    }
    {
        struct TextureParams tex_params = {
            .format=GL_RGBA32F,
            .width=window_width, .height=window_height,
            .generate_mipmap=1, .wrap_s=GL_CLAMP_TO_EDGE,
            .wrap_t=GL_CLAMP_TO_EDGE, .min_filter=GL_LINEAR,
            .mag_filter=GL_LINEAR,
        };
        particles_frame = new_frame(&tex_params, (float *)&vertices_vec[0],
                                    vertices_vec.size()*sizeof(struct Vec2),
                                    NULL, -1);
    }

    int k = 0;
    bool exit_loop = false;
    loop = [&] {
        struct timespec t1, t2;
        // clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t1);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        int steps_per_frame = 25;
        for (int i = 0; i < steps_per_frame; i++) {
            step(&p1_arr[0], &v1_arr[0],
                 &p0_arr[0], &v0_arr[0], t, dt);
                std::swap(force0, force1);
                std::swap(p1_arr, p0_arr);
                std::swap(v1_arr, v0_arr);
                std::swap(densities0, densities1);
                t += dt;
            dt = min(0.0000001, 0.0000001/(velocity_max/2500));
        }
        r = Texture2DData((struct Vec2 *)&p0_arr[0], N_PARTICLES, 1);
        v = Texture2DData((struct Vec2 *)&v0_arr[0], N_PARTICLES, 1);
        glViewport(0, 0, window_width, window_height);
        bind_frame(particles_frame, particles_program);
        set_vertex_attributes(vertex_params, 1);
        r.set_as_sampler2D_uniform("tex");
        set_vec4_uniform("colour", 1.0, 1.0, 1.0, 1.0);
        glDrawArrays(GL_POINTS, 0, N_PARTICLES);
        unbind();
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        set_sampler2D_uniform("tex", particles_frame);
        // v1.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && k > 30)
            exit_loop = true;
        if (glfwWindowShouldClose(window)) {
            exit_status = 1;
            exit_loop = true;
        }
        glfwSwapBuffers(window);
        // clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t2);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        if (k % 2 == 0 && k != 0) {
            // std::cout << "Energy: " << total_energy << std::endl;
            std::cout << "Max velocity: " << velocity_max << std::endl;
            std::cout << "time step: " << dt << std::endl;
            double delta_t = time_difference_in_ms(&t1, &t2);
            // std::cout << delta_t << " ms" << std::endl;
            std::cout << "frames/s: " << (1000.0/delta_t) << std::endl;
            std::cout << "steps/s: "
                        << ((1000.0/delta_t)*steps_per_frame)
                        << std::endl;
        }
        k++;
    };

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
    #else
    while(!exit_loop)
        loop();
    #endif
    return exit_status;
}


static double time_difference_in_ms(const struct timespec *t1,
                                    const struct timespec *t2) {
    if (t2->tv_nsec >= t1->tv_nsec)
        return (double)(t2->tv_nsec - t1->tv_nsec)/1000000.0;
    return (double)(999999999 - t1->tv_nsec + t2->tv_nsec)/1000000.0;
}
