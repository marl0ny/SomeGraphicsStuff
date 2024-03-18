#include "dirac_splitstep_3d.hpp"

// #include <OpenGL/OpenGL.h>
#include <cstdlib>
#include <new>
#define GL_SILENCE_DEPRECATION

#include "texture_data.hpp"
#include "draw_texture_data.hpp"
#include "vector_field_view_3d.hpp"
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

static const double PI = 3.14159;

struct SimulationParams {
    int nx = 64;
    int ny = 64;
    int nz = 64;
    double hbar = 1.0;
    double m = 1.0;
    double dt  = 0.000028;
    double width = 2.0;
    double height = 2.0;
    double length = 2.0;
    double c = 137.036;
    double dx;
    double dy;
    double dz;
    struct InitWavePacket {
        double a = 10.0;
        double sx = 0.07, sy = 0.07, sz = 0.07;
        double bx = 0.5, by = 0.5, bz = 0.5;
        double nx = 10.0, ny = 10.0, nz = 10.0;
    } init_wave_packet;
} sim_params;

static double time_difference_in_ms(const struct timespec *t1,
                                    const struct timespec *t2) {
    if (t2->tv_nsec >= t1->tv_nsec)
        return (double)(t2->tv_nsec - t1->tv_nsec)/1000000.0;
    return (double)(999999999 - t1->tv_nsec + t2->tv_nsec)/1000000.0;
}

/* Numerically solve the Dirac equation in 3D using the split-operator method.

References:

Dirac Equation:

 - Wikipedia contributors. (2023, February 6). 
   Dirac equation. In Wikipedia, The Free Encyclopedia.
   https://en.wikipedia.org/wiki/Dirac_equation

 - Wikipedia contributors. (2023, February 25).
   Dirac spinor. In Wikipedia, The Free Encyclopedia.
   https://en.wikipedia.org/wiki/Dirac_spinor

 - Shankar, R. (1994). The Dirac Equation. 
   In Principles of Quantum Mechanics, chapter 20. Springer.

 - Peskin, M., Schroeder, D. (1995). The Dirac Field.
   In An Introduction to Quantum Field Theory, chapter 3.
   CRC Press.

Split-Operator Method:

 - James Schloss. The Split-Operator Method.
   In The Arcane Algorithm Archive.
   https://www.algorithm-archive.org/contents/
   split-operator_method/split-operator_method.html
 
 - Wikipedia contributors. (2023, January 25). 
   Split-step method. In Wikipedia, The Free Encyclopedia.
   https://en.wikipedia.org/wiki/Split-step_method

 - Bauke, H., Keitel, C. (2011).
   Accelerating the Fourier split operator method 
   via graphics processing units.
   Computer Physics Communications, 182(12), 2454-2463. 
   https://doi.org/10.1016/j.cpc.2011.07.003

Spin and Pauli Spin Matrices:

 - Shankar, R. (1994). Spin.
   In Principles of Quantum Mechanics, chapter 14. Springer.
 
 - Wikipedia contributors. (2023, March 6). 
   Pauli matrices. In Wikipedia, The Free Encyclopedia.
   https://en.wikipedia.org/wiki/Pauli_matrices

*/
static void time_step(Texture2DData &u, Texture2DData &v,
                      const Texture2DData &potential1,
                      const Texture2DData &potential2,
                      DrawTexture2DData &kinetic_prop_drawer,
                      DrawTexture2DData &spatial_prop_drawer,
                      const struct SimulationParams &sim_params) {
    // Apply the position space propagator first pass
    spatial_prop_drawer.set_int_uniform("spinorIndex", 0);
    auto u1 = funcs3D::zeroes(
        COMPLEX2, sim_params.nx, sim_params.ny, sim_params.nz);
    spatial_prop_drawer.draw(u1, 
                        "uTex", u, "vTex", v, "potentialTex", potential1);
    spatial_prop_drawer.set_int_uniform("spinorIndex", 1);
    auto v1 = funcs3D::zeroes(
        COMPLEX2, sim_params.nx, sim_params.ny, sim_params.nz);
    spatial_prop_drawer.draw(v1, 
                        "uTex", u, "vTex", v, "potentialTex", potential1);
    // Fourier transform to momentum space
    auto u2 = funcs3D::fft(u1);
    auto v2 = funcs3D::fft(v1);
    // Apply the kinetic propagator
    kinetic_prop_drawer.set_int_uniform("spinorIndex", 0);
    auto u3 = funcs3D::zeroes(
        COMPLEX2, sim_params.nx, sim_params.ny, sim_params.nz);
    kinetic_prop_drawer.draw(u3, 
                        "uTex", u2, "vTex", v2);
    kinetic_prop_drawer.set_int_uniform("spinorIndex", 1);
    auto v3 = funcs3D::zeroes(
        COMPLEX2, sim_params.nx, sim_params.ny, sim_params.nz);
    kinetic_prop_drawer.draw(v3, 
                        "uTex", u2, "vTex", v2);
    // Inverse fourier transform back to position space
    auto u4 = funcs3D::ifft(u3);
    auto v4 = funcs3D::ifft(v3);
    // Position space propagator final pass
    spatial_prop_drawer.set_int_uniform("spinorIndex", 0);
    spatial_prop_drawer.draw(u, 
                        "uTex", u4, "vTex", v4, "potentialTex", potential2);
    spatial_prop_drawer.set_int_uniform("spinorIndex", 1);
    spatial_prop_drawer.draw(v, 
                        "uTex", u4, "vTex", v4, "potentialTex", potential2);
}

static Texture2DData get_texture_coordinate(
    int orientation_index, SimulationParams sim_params) {
    Texture2DData (* funcs[3])(
        double, double, int, int, int, int, 
        GLuint, GLuint, GLuint, GLuint
        ) = {&funcs3D::make_x, &funcs3D::make_y, &funcs3D::make_z};
    return funcs[orientation_index](
        0.0, 1.0, FLOAT, 
        sim_params.nx, sim_params.ny, sim_params.nz,
        GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR
        ).cast_to(COMPLEX2, X, NONE, X, NONE);
}

static Texture2DData get_initial_wavepacket(const Texture2DData &x, 
                                            const Texture2DData &y,
                                            const Texture2DData &z, 
                                            const SimulationParams &params) {
    auto imag_unit = std::complex<double>(0.0, 1.0);
    double a = sim_params.init_wave_packet.a;
    double sx = sim_params.init_wave_packet.sx;
    double sy = sim_params.init_wave_packet.sy;
    double sz = sim_params.init_wave_packet.sz;
    double bx = sim_params.init_wave_packet.bx;
    double by = sim_params.init_wave_packet.by;
    double bz = sim_params.init_wave_packet.bz;
    double nx = sim_params.init_wave_packet.nx;
    double ny = sim_params.init_wave_packet.ny;
    double nz = sim_params.init_wave_packet.nz;
    auto xb = x - bx;
    auto yb = y - by;
    auto zb = z - bz;
    return a*exp(2.0*PI*imag_unit*(nx*xb + ny*yb + nz*zb))
        *exp(-0.5*(xb*xb/(sx*sx) + yb*yb/(sy*sy) + zb*zb/(sz*sz)));
    
}

int dirac_splitstep_3d(GLFWwindow *window, frame_id main_frame) {

    int exit_status = 0;

    // Get the screen dimensions
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);

    auto imag_unit = std::complex<double>(0.0, 1.0);

    auto kinetic_prop_drawer 
        = DrawTexture2DData(Path("./shaders/dirac-kinetic.frag"));
    kinetic_prop_drawer.set_int_uniform("numberOfDimensions", 3);
    kinetic_prop_drawer.set_ivec3_uniform(
        "texelDimensions3D", 
        {{{.x=sim_params.nx, .y=sim_params.ny, .z=sim_params.nz}}});
    kinetic_prop_drawer.set_vec3_uniform(
        "dimensions3D", 
        {{{.x=(float)sim_params.width, 
                .y=(float)sim_params.height, 
                .z=(float)sim_params.length}}});
    kinetic_prop_drawer.set_float_uniform("dt", sim_params.dt);
    kinetic_prop_drawer.set_float_uniform("m", sim_params.m);
    kinetic_prop_drawer.set_int_uniform("representation", 0);

    auto spatial_prop_drawer
        = DrawTexture2DData(Path("./shaders/dirac-potential.frag"));
    spatial_prop_drawer.set_float_uniform("dt", sim_params.dt/2.0);

    {
        auto drawers = {&kinetic_prop_drawer, &spatial_prop_drawer};
        for (auto &e: drawers) {
            e->set_float_uniform("c", sim_params.c);
            e->set_float_uniform("hbar", sim_params.hbar);
        }
    }

    auto current_drawer
        = DrawTexture2DData(Path("./shaders/dirac-current.frag"));
    current_drawer.set_vec4_uniform("sigmaX", {{{0.0, 0.0, 1.0, 0.0}}});
    current_drawer.set_vec4_uniform("sigmaY", {{{0.0, 0.0, 0.0, -1.0}}});
    current_drawer.set_vec4_uniform("sigmaZ", {{{1.0, -1.0, 0.0, 0.0}}});
    current_drawer.set_int_uniform("representation", 0);

    glViewport(0, 0, sim_params.nx*sim_params.nz, sim_params.ny);

    auto x = get_texture_coordinate(0, sim_params);
    auto y = get_texture_coordinate(1, sim_params);
    auto z = get_texture_coordinate(2, sim_params);

    auto pot 
        = 100.0*powf(64.0F/(float)sim_params.nz, 2.0)
            *((x - 0.5)*(x - 0.5) 
              + (y - 0.5)*(y - 0.5) 
              + (z - 0.5)*(z - 0.5)
            ).cast_to(FLOAT4, NONE, NONE, NONE, X);
    auto u = get_initial_wavepacket(x, y, z, sim_params);
    auto v = 0.0*u;

    auto j = funcs3D::zeroes(
        FLOAT4, 
        sim_params.nx, sim_params.ny, sim_params.nz);

    auto vec_view = VectorFieldView3D(
        {window_width, window_height}, 
        {sim_params.nx, sim_params.ny, sim_params.nz});

    /* auto px = funcs3D::fftshift(2.0*PI*x);
    auto py = funcs3D::fftshift(2.0*PI*y);
    auto pz = funcs3D::fftshift(2.0*PI*z);*/

    int view_program = make_quad_program("./shaders/view.frag");
    int copy_program = make_quad_program("./shaders/copy.frag");

    int k = 0;
    bool exit_loop = false;
    loop = [&] {

        struct timespec frame_start, frame_end;
        clock_gettime(CLOCK_MONOTONIC, &frame_start);

        glViewport(0, 0, sim_params.nx*sim_params.nz, sim_params.ny);
        int steps_per_frame = 1;
        for (int i = 0; i < steps_per_frame; i++) {
            time_step(u, v, pot, pot, 
                      kinetic_prop_drawer, spatial_prop_drawer, sim_params);
        }

        // auto display = u;
        // glViewport(0, 0, window_width, window_height);
        // bind_quad(main_frame, view_program);
        // display.set_as_sampler2D_uniform("tex");
        // draw_unbind_quad();

        current_drawer.draw(j, "uTex", u, "vTex", v);
        auto j0 = j.cast_to(FLOAT3, W, W, W);
        auto vec_display = vec_view.render(0.1*j0, j, 1.5, {0.0, 0.0, 0.0, 1.0});
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, copy_program);
        vec_display.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();

        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && k > 30)
            exit_loop = true;
        if (glfwWindowShouldClose(window)) {
            exit_status = 1;
            exit_loop = true;
        }

        k++;
        glfwSwapBuffers(window);
        clock_gettime(CLOCK_MONOTONIC, &frame_end);
        if (k % 10 == 0 && k != 0) {
            double delta_t = time_difference_in_ms(&frame_start, &frame_end);
	    // std::cout << psi2.sum_reduction().as_dvec4.x << std::endl;
            std::cout << "frames/s: " << (1000.0/delta_t) << std::endl;
            std::cout << "steps/s: " << (1000.0/delta_t)*steps_per_frame
                      << std::endl;
        }
    };

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
    #else
    while (!exit_loop)
        loop();
    #endif

    return exit_status;

}