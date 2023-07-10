#include "mol_dynamics.hpp"

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


static const int N_PARTICLES = 2048;


struct ThreadData {
    int start;
    int end;
    int n_particles;
    float epsilon, sigma;
    const struct Vec2 *positions;
    struct Vec2 *forces;
    float *energies;
};

static float
get_ext_wall_energy(struct Vec2 r, float wall,
                    float lw, float rw, float uw, float dw) {
    float left_wall_e
        = (r.x < lw)? 0.5*wall*pow(r.x - lw, 2.0): 0.0;
    float right_wall_e
        = (r.x > rw)? 0.5*wall*pow(r.x - rw, 2.0): 0.0;
    float ceil_e
        = (r.y > uw)? 0.5*wall*pow(r.y - uw, 2.0): 0.0;
    float floor_e
        = (r.y < dw)? 0.5*wall*pow(r.y - dw, 2.0): 0.0;
    return left_wall_e + right_wall_e + ceil_e + floor_e;
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

static void *threaded_compute_energies_and_forces(void *void_data) {
    struct ThreadData *data = (struct ThreadData *)void_data;
    int n_particles = data->n_particles;
    float epsilon = data->epsilon;
    float sigma = data->sigma;
    float sigma2 = sigma*sigma;
    float sigma6 = sigma2*sigma2*sigma2;
    float sigma12 = sigma6*sigma6;
    float check_dist = 3.0;
    float check_dist3 = check_dist*check_dist*check_dist;
    float check_dist6 = check_dist3*check_dist3;
    float check_dist12 = check_dist6*check_dist6;
    float e_offset
         = 4.0*epsilon*(1.0/check_dist12 - 1.0/check_dist6);
    for (int j = data->start; j < data->end; j++) {
        float rj_x = data->positions[j].x;
        float rj_y = data->positions[j].y;
        struct Vec2 rj {rj_x, rj_y};
        struct Vec2 int_force {0.0, 0.0};
        float int_energy = 0.0;
        for (int i = j+1; i < n_particles; i++) {
            float ri_x = data->positions[i].x;
            float ri_y = data->positions[i].y;
            float rx = ri_x - rj_x;
            float ry = ri_y - rj_y;
            float r2 = rx*rx + ry*ry;
            // int_energy
            //         += 4.0*epsilon*(1.0/check_dist12 - 1.0/check_dist6);
            if (r2 < ((check_dist*check_dist)*sigma2)) {
                float r4 = r2*r2;
                float r6 = r4*r2;
                float r8 = r4*r4;
                float r12 = r8*r4;
                float r14 = r12*r2;
                float s1 = (4.0*epsilon)*(-12.0*sigma12/r14);
                float s2 = (4.0*epsilon)*(6.0*sigma6/r8);
                int_energy += 4.0*epsilon*(sigma12/r12 - sigma6/r6) - e_offset;
                float fx = rx*(s1 + s2);
                float fy = ry*(s1 + s2);
                data->forces[j].x += fx;
                data->forces[j].y += fy;
                data->forces[i].x -= fx;
                data->forces[i].y -= fy;
            }
        }
    }
    return NULL;
}

#define MAX_SUPPORTED_THREADS 32
static int number_of_threads_to_use() {
    // Number of threads to use
    // https://stackoverflow.com/a/150971
#ifndef WIN_32
    int n_threads = sysconf(_SC_NPROCESSORS_ONLN);
#else
    int n_threads = 4
#endif
        if (n_threads > MAX_SUPPORTED_THREADS)
            return MAX_SUPPORTED_THREADS;
    return n_threads;
}

static double time_difference_in_ms(const struct timespec *t1,
                                    const struct timespec *t2) {
    if (t2->tv_nsec >= t1->tv_nsec)
        return (double)(t2->tv_nsec - t1->tv_nsec)/1000000.0;
    return (double)(999999999 - t1->tv_nsec + t2->tv_nsec)/1000000.0;
}

/* A very rudimentary molecular dynamics simulation.
 * The primary reference for this is the
 * Interactive Molecular Dynamics program by Daniel Schroeder
 * found here https://physics.weber.edu/schroeder/md/,
 * as well as this accompanying article:
 * https://physics.weber.edu/schroeder/md/InteractiveMD.pdf.
 * For writing multithreaded code using the Posix thread library,
 * this set of page was helpful:
 * https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html.
 *
 * References:
 *
 *  - Schroeder, D. Interactive Molecular Dynamics
 *    https://physics.weber.edu/schroeder/md/
 *
 *  - Schroeder, D. (2015). Interactive Molecular Dynamics.
 *    American Journal of Physics, 83(3), 210 - 218.
 *    http://dx.doi.org/10.1119/1.4901185
 *
 *  - Ippolito, G. (2004). Posix thread (pthread) libraries.
 *    YoLinux Tutorials.
 *    https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/
 *     //www/pthreads.html.
 *
 * */
int particles_lennard_jones_mt(GLFWwindow *window, frame_id main_frame) {
    int exit_status = 0;
    int window_width {}, window_height {};
    window_dimensions(window, &window_width, &window_height);
    // double dt = 0.000001;
    double t = 0.0;
    double velocity_max = 0.0;
    // double dt = 0.0000005;
    double dt = 0.0000001;
    double total_energy = 0.0;
    // double dt = 0.000000001;
    // double dt = 0.0;
    frame_id view_program
        = make_quad_program("./shaders/copy.frag");

    auto p0_arr = std::vector<struct Vec2> {};
    auto v0_arr = std::vector<struct Vec2> {};
    auto p1_arr = std::vector<struct Vec2> {};
    auto v1_arr = std::vector<struct Vec2> {};
    for (int i = 0; i < N_PARTICLES; i++) {
        // float rx = (float)(i % 64)/(float)128 + 0.25;
        // float ry = (float)(i / 64)/(float)128 + 0.25;
        float offset = ((i / 64) % 2 == 1)? 0.5/128: 0.0;
        // float offset = 0.0;
        float rx = (float)(i % 64)/(float)128 + offset + 0.25;
        float ry = (float)(i / 64)/(float)128 + 0.25;
        // float rx = (float)(i % 128)/(float)128 + 0.25;
        // float ry = (float)(i / 128)/(float)128 + 0.25;
        double theta = -0.25;
        float rx2 = cos(theta)*rx - sin(theta)*ry;
        float ry2 = sin(theta)*rx + cos(theta)*ry;
        p0_arr.push_back({.x = rx2, .y = ry2});
        v0_arr.push_back({.x = -5600.0F*ry2, .y = 5600.0F*rx2});
        p1_arr.push_back({.x = rx2, .y = ry2});
        v1_arr.push_back({.x = -5600.0F*ry2, .y = 5600.0F*rx2});
    }

    double sigma = 0.0075;
    // double epsilon = 8000000.0;
    // double epsilon = 10000000.0;
    double epsilon = 2e7;
    // double gravity_force = -800000.0;
    double gravity_force = -10000000.0;
    double wall_force = 1e13;

    auto r = Texture2DData((struct Vec2 *)&p0_arr[0], N_PARTICLES, 1);
    auto v = Texture2DData((struct Vec2 *)&v0_arr[0], N_PARTICLES, 1);
    int n_threads = number_of_threads_to_use();
    std::cout << "Threads used: " << n_threads << std::endl;
    int updates_per_th = N_PARTICLES/n_threads;
    std::vector<std::vector<struct Vec2>> force_th {};
    for (int i = 0; i < n_threads; i++)
        force_th.push_back(std::vector<struct Vec2> {N_PARTICLES});

    std::vector<int> thread_starts = {};
    std::vector<int> thread_ends = {};
    {
        auto quad_root_pos = [](double a, double b, double c) {
            return (-b + sqrt(b*b - 4.0*a*c))/(2.0*a);
        };
        double total_ops = N_PARTICLES*(N_PARTICLES/2.0 + 0.5);
        thread_starts.push_back(0);
        for (int i = n_threads - 1; i >= 1; i--) {
            double root
                = quad_root_pos(0.5, 0.5, 
                -((double)i)*total_ops/((double)n_threads));
            thread_starts.push_back(N_PARTICLES - int(root));
            thread_ends.push_back(N_PARTICLES - int(root));

        }
        thread_ends.push_back(N_PARTICLES);
        for (auto &e: thread_ends) std::cout << e << std::endl;
    }
    std::vector<struct Vec2> force0 {N_PARTICLES};
    std::vector<struct Vec2> force1 {N_PARTICLES};

    auto step = [&] (struct Vec2 *r1, struct Vec2 *v1,
                     const struct Vec2 *r0, const struct Vec2 *v0,
                     double t, double dt) {
        float energies[N_PARTICLES] = {0.0,};
        for (int i = 0; i < N_PARTICLES; i++) {
            force1[i].x = 0.0;
            force1[i].y = 0.0;
            for (int k = 0; k < force_th.size(); k++) {
                force_th[k][i].x = 0.0;
                force_th[k][i].y = 0.0;
            }
        }
        auto threaded_force_update = [&](struct Vec2 *force,
                                         const struct Vec2 *positions) {
            int n_th = 0;
            pthread_t threads[MAX_SUPPORTED_THREADS] = {0,};
            struct ThreadData particle_threads[MAX_SUPPORTED_THREADS] = {};
            for (; n_th < n_threads; n_th++) {
                particle_threads[n_th].start = thread_starts[n_th];
                particle_threads[n_th].end = thread_ends[n_th];
                particle_threads[n_th].n_particles = N_PARTICLES;
                particle_threads[n_th].epsilon = epsilon;
                particle_threads[n_th].sigma = sigma;
                particle_threads[n_th].positions = positions;
                particle_threads[n_th].forces = &force_th[n_th][0];
                particle_threads[n_th].energies = energies;
		// threaded_compute_energies_and_forces((void *)&particle_threads[n_th]);
                pthread_create(&threads[n_th], NULL,
                               threaded_compute_energies_and_forces,
                               (void *)&particle_threads[n_th]);
                // pthread_join(threads[n_th], NULL);
            }
            for (int i = 0; i < n_th; i++) {
                pthread_join(threads[i], NULL);
                for (int j = 0; j < N_PARTICLES; j++) {
                    force[j].x += force_th[i][j].x;
                    force[j].y += force_th[i][j].y;
                    if (i == n_th - 1) {
                        struct Vec2 gravity_force_vec {.x=0.0F, .y=(float)gravity_force};
                        struct Vec2 rj {positions[j]};
                        float ext_wall_energy
                            = get_ext_wall_energy(rj, wall_force,
                                                  0.01, 0.99, 0.99, 0.01);
                        struct Vec2 ext_wall_force
                            = get_ext_wall_force(rj, wall_force,
                                                 0.01, 0.99, 0.99, 0.01);
                        float gravity_energy = -gravity_force*rj.y;
                        force[j].x += ext_wall_force.x + gravity_force_vec.x;
                        force[j].y += ext_wall_force.y + gravity_force_vec.y;
                        energies[j] += ext_wall_energy + gravity_energy;
                    }
                }
            }
        };
        if (t < dt) {
            threaded_force_update(&force0[0], r0);
        }
        for (int i = 0; i < N_PARTICLES; i++) {
            r1[i].x = r0[i].x + v0[i].x*dt + 0.5*dt*dt*force0[i].x;
            r1[i].y = r0[i].y + v0[i].y*dt + 0.5*dt*dt*force0[i].y;
        }
        threaded_force_update(&force1[0], r1);
        double ke = 0.0, pe = 0.0;
        double v2_max = 0.0;
        for (int i = 0; i < N_PARTICLES; i++) {
            v1[i].x = v0[i].x + 0.5*dt*(force0[i].x + force1[i].x);
            v1[i].y = v0[i].y + 0.5*dt*(force0[i].y + force1[i].y);
            float v2 = v1[i].x*v1[i].x + v1[i].y*v1[i].y;
            if (v2_max < v2)
                v2_max = v2; 
            ke += v2/2.0;
            pe += energies[i];
        }
        velocity_max = sqrt(v2_max);
        total_energy = ke + pe;
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

    for (int k = 0, exit_loop=false; !exit_loop; k++) {
        struct timespec t1, t2;
        // clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t1);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        int steps_per_frame = 25;
        for (int i = 0; i < steps_per_frame; i++)
        {
            step((struct Vec2 *)&p1_arr[0], (struct Vec2 *)&v1_arr[0],
                 (struct Vec2 *)&p0_arr[0], (struct Vec2 *)&v0_arr[0],
                 t, dt);
            std::swap(force0, force1);
            std::swap(p1_arr, p0_arr);
            std::swap(v1_arr, v0_arr);
            t += dt;
        }
        dt = 0.0000001/(velocity_max/2500);
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
            std::cout << "energy: " << total_energy << std::endl;
            std::cout << "max velocity: " << velocity_max << std::endl;
            std::cout << "time step: " << dt << std::endl;
            double delta_t = time_difference_in_ms(&t1, &t2);
            // std::cout << delta_t << " ms" << std::endl;
            std::cout << "frames/s: " << (1000.0/delta_t) << std::endl;
            std::cout << "steps/s: "
                << ((1000.0/delta_t)*steps_per_frame)
                << std::endl;
        }
    }
    return exit_status;
}
