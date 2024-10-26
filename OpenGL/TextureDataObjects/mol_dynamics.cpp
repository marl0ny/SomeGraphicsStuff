/* A very rudimentary molecular dynamics simulation.
 * This is based on the
 * Interactive Molecular Dynamics program by Daniel Schroeder
 * found here https://physics.weber.edu/schroeder/md/,
 * as well as this accompanying article:
 * https://physics.weber.edu/schroeder/md/InteractiveMD.pdf.
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
 * */
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
#include <time.h>

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

static double time_difference_in_ms(const struct timespec *t1,
                                    const struct timespec *t2) {
    if (t2->tv_nsec >= t1->tv_nsec)
        return (double)(t2->tv_nsec - t1->tv_nsec)/1000000.0;
    return (double)(999999999 - t1->tv_nsec + t2->tv_nsec)/1000000.0;
}

int particles_lennard_jones(Renderer *renderer) {
    int main_frame = renderer->main_frame;
    GLFWwindow *window = renderer->window;
    int exit_status = 0;
    int window_width {}, window_height {};
    window_dimensions(window, &window_width, &window_height);
    int N_PARTICLES = 2048;
    // double dt = 0.000001;
    double t = 0.0;
    double dt = 0.0000005;
    // double dt = 0.00000001;
    // double dt = 0.0;
    frame_id view_program
        = make_quad_program("./shaders/util/copy.frag");

    auto p0_arr = std::vector<struct Vec2> {};
    auto v0_arr = std::vector<struct Vec2> {};
    for (int i = 0; i < N_PARTICLES; i++) {
        float rx = (float)(i % 64)/(float)128 + 0.25;
        float ry = (float)(i / 64)/(float)128 + 0.25;
        // float rx = (float)(i % 128)/(float)128 + 0.25;
        // float ry = (float)(i / 128)/(float)128 + 0.25;
        double theta = -0.25;
        float rx2 = cos(theta)*rx - sin(theta)*ry;
        float ry2 = sin(theta)*rx + cos(theta)*ry;
        p0_arr.push_back({.x = rx2, .y = ry2});
        v0_arr.push_back({.x = -700.0F*ry2, .y = 700.0F*rx2});
    }

    double sigma = 0.0075;
    double epsilon = 100000.0;
    double gravity_force = -30000.0;
    double wall_force = 1e10;
    double total_energy = 0.0;

    auto r = Texture2DData((struct Vec2 *)&p0_arr[0], N_PARTICLES, 1);
    auto v = Texture2DData((struct Vec2 *)&v0_arr[0], N_PARTICLES, 1);

    auto energy_force_for_com
        = DrawTexture2DData(Path("./shaders/mol-dynamics/lennard-jones-for.frag"));
    energy_force_for_com.set_float_uniforms({{"sigma", sigma},
                                             {"epsilon", epsilon},
                                             {"nParticles", (float)N_PARTICLES},
                                             {"gravity", gravity_force},
                                             {"wall", wall_force}});
    energy_force_for_com.set_int_uniforms({{"isRow", true}});

    Texture2DData step_data = funcs2D::zeroes(FLOAT4, N_PARTICLES, 1);
    auto step = [&](const Texture2DData &r0, const Texture2DData &v0,
                              double dt, double t,
                              int i) -> std::vector<Texture2DData> {
        glViewport(0, 0, N_PARTICLES, 1);

        if (t < dt)
            energy_force_for_com.draw(step_data, "positionsTex", r0);
        Texture2DData force0 = step_data.cast_to(FLOAT2, X, Y);
        Texture2DData r1 = r0 + v0*dt + (0.5*dt*dt)*force0;
        energy_force_for_com.draw(step_data, "positionsTex", r1);
        Texture2DData force1 = step_data.cast_to(FLOAT2, X, Y);
        Texture2DData v1 = v0 + 0.5*(force0 + force1)*dt;
        Texture2DData pot_energies = step_data.cast_to(FLOAT, Z);
        double pe = pot_energies.sum_reduction().as_double;
        double ke = 0.5*v1.squared_norm().as_double;
        total_energy = pe + ke;

        return std::vector<Texture2DData> {r1, v1};
    };

    // int NX = 256, NY = 256;

    // Particles view
    int particles_program = make_program(
        "./shaders/particles/particles.vert",
        "./shaders/util/colour.frag");
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
        clock_gettime(CLOCK_MONOTONIC, &t1); 
	// clock_gettime(CLOCK_REALTIME, &t1);
        int steps_per_frame = 25;
        for (int i = 0; i < steps_per_frame; i++)
        {
            std::vector<Texture2DData> tmp = step(r, v, dt, t, i);
            t += dt;
            r = tmp[0], v = tmp[1];
        }
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
	clock_gettime(CLOCK_MONOTONIC, &t2);
        // clock_gettime(CLOCK_REALTIME, &t2);
        if (k % 10 == 0 && k != 0) {
            std::cout << "Energy: " << total_energy << std::endl;
            double delta_t = time_difference_in_ms(&t1, &t2);
            // std::cout << delta_t << " ms" << std::endl;
            std::cout << "frames/s: " 
                      << floor((1000.0/delta_t)) 
                      << std::endl;
            std::cout << "steps/s: " 
                << floor((1000.0/delta_t)*steps_per_frame) 
                << std::endl;
        }
        k++;
    };
    
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
    #else
    while (!exit_loop) loop();
    #endif
    return exit_status;
}
