#include "stable_fluids.hpp"

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

/*
Stable fluids simulation.

References:
 - Stam, J. (July 1999). Stable Fluids.
   SIGGRAPH99: 26th International Conference on
   Computer Graphics and Interactive Techniques, 121-128.
   https://doi.org/10.1145/311535.311548
 - Harris, M. (2004). Fast Fluid Dynamics Simulation on the GPU.
   In GPU Gems, chapter 38. NVIDIA.
   https://developer.nvidia.com/gpugems/gpugems/
   part-vi-beyond-triangles/chapter-38-fast-fluid-dynamics-simulation-gpu
 - Crane, K., Llamas, I., Tariq, S. (2007).
   Real-Time Simulation and Rendering of 3D Fluids.
   In GPU Gems 3, chapter 30. NVIDIA.
   https://developer.nvidia.com/gpugems/gpugems3/
   part-v-physics-simulation/
   chapter-30-real-time-simulation-and-rendering-3d-fluids
*/
int stable_fluids(GLFWwindow *window, frame_id main_frame) {
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    int NX = 256, NY = 256;
    double dt = 1.0;
    double width = (double)NX, height = (double)NY;
    double dx = width/(double)NX, dy = height/(double)NY;
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    auto advect_com = DrawTexture2DData(Path("./shaders/isf-bw-advect.frag"));
    advect_com.set_float_uniforms({{"width", width}, {"height", height}});
    auto advect_higher_or_com
        = DrawTexture2DData(Path("./shaders/advect-higher-or.frag"));
    advect_higher_or_com.set_float_uniforms({{"width", width},
                                             {"height", height},
                                             {"dx", dx}, {"dy", dy},
                                             {"dt", dt}});
    // getchar();
    auto poisson_solve_com
        = DrawTexture2DData(Path("./shaders/poisson-jacobi.frag"));

    auto init_dist_com
         = DrawTexture2DData(Path("./shaders/isf-init-dist.frag"));

    init_dist_com.set_float_uniforms({{"amplitude", 3.0},
                                      {"sigma", 0.1}});
    init_dist_com.set_vec2_uniforms({{"r0", {.x=0.25, .y=0.25}}});
    auto dist0 = init_dist_com.create(FLOAT, NX, NY);

    auto init_vel_com
         = DrawTexture2DData(Path("./shaders/stable-fluids-init-vel.frag"));
    init_vel_com.set_float_uniforms({{"amplitude", 5.0},
                                      {"sigma", 0.07}});
    init_vel_com.set_vec2_uniforms({{"r0", {.x=0.25, .y=0.25}},
                                    {"v0", {.x=1.0, .y=1.1}}});
    auto vel0 = init_vel_com.create(FLOAT2, NX, NY);

    init_dist_com.set_vec2_uniforms({{"r0", {.x=0.75, .y=0.75}}});
    init_dist_com.set_float_uniforms({{"amplitude", -3.0}});
    auto dist1 = init_dist_com.create(FLOAT, NX, NY);

    init_vel_com.set_vec2_uniforms({{"r0", {.x=0.75, .y=0.75}},
                                    {"v0", {.x=-1.0, .y=-1.0}}});
    auto vel1 = init_vel_com.create(FLOAT2, NX, NY);

    auto dist = dist0 + dist1;
    auto vel = vel0 + vel1;

    auto view_com
        = DrawTexture2DData(Path("./shaders/view.frag"));
    auto *data = new uint8_t [window_width*window_height*3] {0,};

    auto advect_func = [&](Texture2DData &dist,
                                     Texture2DData &v) -> Texture2DData {
        advect_com.set_float_uniforms({{"dt", dt}});
        auto forward = 0.0*dist;
        // advect_com.draw(forward, "velocityTex", v, "densityTex", dist);
        // return forward;
        advect_com.draw(forward, "velocityTex", v, "densityTex", dist);
        advect_com.set_float_uniforms({{"dt", -dt}});
        auto backward = 0.0*dist;
        advect_com.draw(backward,
                        "velocityTex", v, "densityTex", forward);
        auto out = 0.0*dist;
        advect_higher_or_com.draw(out,
                                  "tex", dist,
                                  "velocityTex", v,
                                  "advectForwardTex", forward,
                                  "advectReverseTex", backward);
        return out;
    };
    auto pressure_func = [&](Texture2DData &vel,
                             Texture2DData &pressure0,
                             int n_iterations) -> Texture2DData {
        struct Grad2DParams grad_params {
            .dx=dx, .dy=dy, .width=width, .height=height,
            .staggered=1, .order_of_accuracy=4
        };
        auto div_dot_v = funcs2D::ddx(vel, grad_params).cast_to(FLOAT, X)
                                + funcs2D::ddy(vel, 
                                               grad_params).cast_to(FLOAT, Y);
        poisson_solve_com.set_float_uniforms({{"dx", dx}, {"dy", dy},
                                              {"width", width},
                                              {"height", height}});
        auto x = pressure0;
        for (int i = 0; i < n_iterations; i++) {
            poisson_solve_com.draw(x,
                             "prevIterTex", x, "bTex", div_dot_v);
        }
        return x;
    };
    auto pressure_project_func = [&](Texture2DData &v,
                                     Texture2DData &p
                                    ) -> Texture2DData {
        Grad2DParams grad_params {
            .dx=dx, .dy=dy, .width=width, .height=height,
            .staggered=-1, .order_of_accuracy=4,
        };
        auto dpdx = funcs2D::ddx(p, grad_params);
        auto dpdy = funcs2D::ddy(p, grad_params);
        auto grad_p = dpdx.cast_to(FLOAT2, X, NONE)
                                     + dpdy.cast_to(FLOAT2, NONE, X);
        return v - grad_p;
    };

    auto pressure = 0.0*dist;

    int k = 0;
    bool exit_loop = false;
    loop = [&] {
        glViewport(0, 0, NX, NY);
        dist = advect_func(dist, vel);
        vel = advect_func(vel, vel);
        pressure = pressure_func(vel, pressure, 15);
        vel = pressure_project_func(vel, pressure);
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        dist.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        /*{
            auto bmp_frame1 = Texture2DData(BYTE3, window_width, window_height);
            view_com.draw(bmp_frame1, "tex", dist);
            auto bmp_frame2 = bmp_frame1.cast_to(BYTE3, B, G, R);
            bmp_frame2.paste_to_array(data);
            std::stringstream filename {};
            filename << "data_" << k << ".bmp";
            std::string filename_str = filename.str();
            write_to_bitmap(&filename_str[0], data,
                            window_width, window_height);
                            }*/
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && k > 30)
            exit_loop = true;
        if (glfwWindowShouldClose(window)) {
            exit_status = 1;
            exit_loop = true;
        }
        glfwSwapBuffers(window);
        k++;
    };

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
    #else
    while (!exit_loop)
        loop();
    #endif
    
    return exit_status;
}
