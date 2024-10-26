/* Very basic Maxwell simulation in 3D using the FDTD method.

References:

 - Taflove A., Hagness S. (2000).
   Introduction to Maxwell's Equations and the Yee Algorithm.
   In Computational Electrodynamics: 
   The Finite-Difference Time-Domain Method, 
   chapter 3. Artrech House.

 - Wikipedia - Finite-difference time-domain method
   https://en.wikipedia.org/wiki/Finite-difference_time-domain_method

*/
#include "electrodynamics_3d.hpp"

// #include <OpenGL/OpenGL.h>
#include <cstdlib>
#include <new>
#include <cmath>
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
#include "vector_field_view_3d.hpp"

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

static struct SimulationParams {
    int nx = 64;
    int ny = 64;
    int nz = 64;
    double hbar = 1.0;
    double m = 1.0;
    double dt  = 2.0e-11;
    double width = 64.0;
    double height = 64.0;
    double length = 64.0;
    double c = 300000000;
    double dx;
    double dy;
    double dz;
    // Constants from pg 69 of Taflove
    double epsilon = 8.854e-12; // Electric permittivity
    double sigma = 0.0;  // electric conductivity
    double mu = 4.0*PI*1e-7; // magnetic permeability
    double sigma_m = 0.0; // magnetic loss
    struct InitWavePacket {
        double a = 50.0;
        double sx = 0.03, sy = 0.03, sz = 0.03;
        double bx = 0.5, by = 0.25, bz = 0.5;
        double nx = 10.0, ny = 10.0, nz = 10.0;
    } init_wave_packet;
} sim_params;


static double time_difference_in_ms(const struct timespec *t1,
                                    const struct timespec *t2) {
    if (t2->tv_nsec >= t1->tv_nsec)
        return (double)(t2->tv_nsec - t1->tv_nsec)/1000000.0;
    return (double)(999999999 - t1->tv_nsec + t2->tv_nsec)/1000000.0;
}

/* Note than when compiling on O0 the struct members of 
sim_params.init_wave_packet do not contain their expected values.
*/
static Texture2DData get_initial_wavepacket(const Texture2DData &x, 
                                            const Texture2DData &y,
                                            const Texture2DData &z, 
                                            const SimulationParams &params) {
    // auto imag_unit = std::complex<double>(0.0, 1.0);
    double a = sim_params.init_wave_packet.a;
    double sx = sim_params.init_wave_packet.sx;
    double sy = sim_params.init_wave_packet.sy;
    double sz = sim_params.init_wave_packet.sz;
    double bx = sim_params.init_wave_packet.bx;
    double by = sim_params.init_wave_packet.by;
    double bz = sim_params.init_wave_packet.bz;
    // double nx = sim_params.init_wave_packet.nx;
    // double ny = sim_params.init_wave_packet.ny;
    // double nz = sim_params.init_wave_packet.nz;
    auto xb = x - bx;
    auto yb = y - by;
    auto zb = z - bz;
    //std::printf("%g %g %g", a, xb, sx);
    // std::cout << a << ", " << sx << ", " << bx << ", " << std::endl;
    return a*exp(-0.5*(xb*xb/(sx*sx) + yb*yb/(sy*sy) + zb*zb/(sz*sz)));    
}

static void time_step(Texture2DData &e0, Texture2DData &e1,
                      Texture2DData &b0, Texture2DData &b1,
                      Texture2DData &j,
                      DrawTexture2DData &curl_draw,
                      DrawTexture2DData &e_draw,
                      DrawTexture2DData &b_draw) {
    auto curl_b0 = 0.0*b0;
    curl_draw.set_int_uniform("sampleOffset", -1);
    curl_draw.draw(curl_b0, "tex", b0);
    e_draw.draw(e1, "texCurlHField", curl_b0, "texEField", e0, "texJ", j);
    swap(e1, e0);
    auto curl_e0 = 0.0*e0;
    curl_draw.set_int_uniform("sampleOffset", 0);
    curl_draw.draw(curl_e0, "tex", e1);
    b_draw.draw(b1, "texCurlEField", curl_e0, "texHField", b0, "texM", j);
    swap(b1, b0);
}

int electrodynamics_3d(Renderer *renderer) {

    GLFWwindow *window = renderer->window;
    int main_frame = renderer->main_frame;


    int exit_status = 0;

    // Get the screen dimensions
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);

    int view_program = make_quad_program("./shaders/view.frag");

    glViewport(0, 0, window_width, window_height);

    struct IVec2 view_dimensions {.x=window_width, .y=window_height};
    struct IVec3 vector_dimensions {.x=sim_params.nx, .y=sim_params.ny, .z=sim_params.nz};
    auto vector_view 
        = VectorFieldView3D(view_dimensions, vector_dimensions);

    use_3d_texture(sim_params.nx, sim_params.ny, sim_params.nz);

    sim_params.dx = sim_params.width/sim_params.nx;
    sim_params.dy = sim_params.height/sim_params.ny;
    sim_params.dz = sim_params.length/sim_params.nz;
    // sim_params.init_wave_packet.ax = 50.0;

    auto charge_draw 
        = DrawTexture2DData(Path("./shaders/fdtd/oscillating-charge.frag"));
    charge_draw.set_float_uniform("phi", 0.0);
    charge_draw.set_float_uniform("omega", 0.1*1.0/sim_params.dt);
    charge_draw.set_float_uniform("chargeAmplitude", 3.0);
    charge_draw.set_vec3_uniform("sigma", {0.03, 0.03, 0.03});
    charge_draw.set_vec3_uniform("dimensions3D", 
        {(float)sim_params.width, 
        (float)sim_params.height,
        (float)sim_params.length});
    charge_draw.set_ivec3_uniform("texelDimensions3D", 
        {sim_params.nx, sim_params.ny, sim_params.nz});

    auto curl_draw = DrawTexture2DData(Path("./shaders/curl/curl3d.frag"));
    curl_draw.set_ivec2_uniform(
        "texelDimensions2D", {sim_params.nx*sim_params.nz, sim_params.ny});
    curl_draw.set_ivec3_uniform("texelDimensions3D",
                                {sim_params.nx, sim_params.ny, sim_params.nz});
    curl_draw.set_vec3_uniform(
        "dr", {(float)sim_params.dx, 
                         (float)sim_params.dy, (float)sim_params.dz});
    curl_draw.set_vec3_uniform(
        "dimensions3D", 
        {(float)sim_params.width, 
              (float)sim_params.height,
              (float)sim_params.length});
    curl_draw.set_int_uniform("orderOfAccuracy", 4);
    // curl_draw.set_int_uniform("samplerOffset", 0);

    auto e_draw 
        = DrawTexture2DData(Path("./shaders/fdtd/electric3d.frag"));
    e_draw.set_float_uniforms(
        {{"dt", sim_params.dt},
         {"epsilon", sim_params.epsilon}, 
         {"sigma", sim_params.sigma}});

    auto b_draw 
        = DrawTexture2DData(Path("./shaders/fdtd/magnetic3d.frag"));
    b_draw.set_float_uniforms(
        {{"dt", sim_params.dt},
         {"mu", sim_params.mu}, 
         {"sigma", sim_params.sigma_m}});


    auto x = funcs3D::make_x(
        0.0, 1.0, FLOAT,
        sim_params.width, sim_params.height, sim_params.length);
    auto y = funcs3D::make_y(
        0.0, 1.0, FLOAT,
        sim_params.width, sim_params.height, sim_params.length);
    auto z = funcs3D::make_z(
        0.0, 1.0, FLOAT,
        sim_params.width, sim_params.height, sim_params.length);

    auto wave = get_initial_wavepacket(x, y, z, sim_params);
    auto e_field0 = wave.cast_to(FLOAT4, X, X, X, NONE);
    auto b_field0 = e_field0*0.0;
    auto e_field1 = e_field0*1.0;
    auto b_field1 = 0.0*b_field0;
    auto j = 0.0*e_field0;

    frame_id copy_program = make_quad_program("./shaders/util/copy.frag");
    


    int k = 0;
    bool exit_loop = false;
    loop = [&] {

        int steps_per_frame = 20;
        struct timespec frame_start, frame_end;
        clock_gettime(CLOCK_MONOTONIC, &frame_start);

        use_3d_texture(sim_params.nx, sim_params.ny, sim_params.nz);
        for (int i = 0; i < steps_per_frame; i++)
            time_step(e_field0, e_field1, 
                b_field0, b_field1, j, curl_draw, e_draw, b_draw);

        auto field_vis1 
            = e_field0.cast_to(FLOAT4, X, NONE, NONE, NONE);
        auto field_vis2 
            = e_field0.cast_to(FLOAT4, NONE, NONE, X, NONE);
        auto ones 
            = (funcs3D::make_x(0.0, 1.0, FLOAT, 
            sim_params.nx, sim_params.ny, sim_params.nz) + 1.0
            ).cast_to(FLOAT4, NONE, NONE, NONE, X);
        auto field_vis0 = field_vis1 - field_vis2;
        auto field_vis = field_vis0 + ones;

        glViewport(0, 0, window_width, window_height);
        // auto t
        //      = vector_view.render(field_vis, e_field0, 
        // 1.0, {0.0, 0.0, 1.0F/sqrtf(2.0), 1.0F/sqrtf(2.0)});
        auto t
             = vector_view.render(field_vis, e_field0, 
        1.0, {0.0, 0.0, 0.0, 1.0});
        /* bind_quad(main_frame, view_program);
        field_vis.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();*/
        bind_quad(main_frame, copy_program);
        t.set_as_sampler2D_uniform("tex");
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
        if (k % 1000000 == 0 && k != 0) {
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
