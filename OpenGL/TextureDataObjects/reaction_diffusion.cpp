#include "reaction_diffusion.hpp"

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
Gray Scott Reaction Diffusion.

The following projects were used as references:

 - Abelson, Adams, Coore, Hanson, Nagpul, Sussmann.
   Gray Scott Model of Reaction Diffusion.
   https://groups.csail.mit.edu/mac/projects/amorphous/GrayScott/

 - Käfer, K., Schulz M.
   Gray-Scott Model of a Reaction-Diffusion System
   Pattern Formation.
   https://itp.uni-frankfurt.de/~gros/StudentProjects/
	 //Projects_2020/projekt_schulz_kaefer/

*/
int gray_scott_reaction_diffusion(GLFWwindow *window,
                                   frame_id main_frame) {
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    int NX = 128, NY = 128;
    // u_t = r_u*del^2 u - u*v^2 + f*(1 - u)
    // v_t = r_v*del^2 v + u*v^2 - f*v - k*v
    // double k = 0.06;
    // double f = 0.04;
    double k = 0.06;
    double f = 0.044;
    double dt = 0.05;
    // double r_u = 0.01, r_v = 0.01;
    double r_u = 0.05;
    double r_v = 0.5*r_u;
    double width = (double)NX;
    double height = (double)NY;
    double dx = width/(double)NX;
    double dy = height/(double)NY;
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    std::vector<Texture2DData> uv;
    {
        auto x = funcs2D::make_x(-0.5, 0.5, FLOAT, NX, NY);
        auto y = funcs2D::make_y(-0.5, 0.5, FLOAT, NX, NY);
        auto u = exp(-0.5*(x*x + y*y)/(0.03*0.03));
        {
            // u = u - 3.0*exp(-0.5*(pow(x - 0.25, 2)
            //                          + pow(y - 0.25, 2))/(0.05*0.05));
            u = u + exp(-0.5*(pow(x + 0.25, 2)
                              + pow(y + 0.25, 2))/(0.03*0.03));
        }
        auto v = funcs2D::roll(u, {.x=0.02, .y=0.02});
        uv = {u, v};
    }

    auto eom = [&](const Texture2DData &u,
                             const Texture2DData &v
                            ) -> std::vector<Texture2DData> {
        Laplacian2DParams params {
            .dx=dx, .dy=dy, .width=width, .height=height, .order_of_accuracy=4,
        };
        return {
            r_u*funcs2D::laplacian(u, params) - u*pow(v, 2) + f*(1.0 - u),
            r_v*funcs2D::laplacian(v, params) + u*pow(v, 2) - (k + f)*v
        };
    };

    auto forward_euler = [&](const Texture2DData &u,
                                  const Texture2DData &v,
                                  double dt
                                 ) -> std::vector<Texture2DData> {
        auto uv2 = eom(u, v);
        return {
            u + dt*uv2[0],
            v + dt*uv2[1]
        };
    };

    auto rk4 = [&](const Texture2DData &u,
                                  const Texture2DData &v,
                                  double dt
                                 ) -> std::vector<Texture2DData> {
        std::vector<Texture2DData> uv1, uv2, uv3, uv4;
        {
            uv1 = eom(u, v);
            uv2 = eom(u + uv1[0]*(dt/2.0), v + uv1[1]*(dt/2.0));
            uv3 = eom(u + uv2[0]*(dt/2.0), v + uv2[1]*(dt/2.0));
            uv4 = eom(u + uv3[0]*dt, v + uv3[1]*dt);
        };
        return {
            u + (uv1[0] + 2.0*uv2[0] + 2.0*uv3[0] + uv4[0])*(dt/6.0),
            v + (uv1[1] + 2.0*uv2[1] + 2.0*uv3[1] + uv4[1])*(dt/6.0)
        };
    };

    bool exit_loop = false;
    int n = 0;
    loop = [&] {
        glViewport(0, 0, width, height);
        for (int k = 0; k < 50; k++) {
            uv = forward_euler(uv[0], uv[1], dt);
            // uv = rk4(uv[0], uv[1], dt);
        }
        // uv = rk4(uv[0], uv[1], dt);
        auto view_u = 1.0*uv[0];
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        view_u.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && n > 30)
            exit_loop = true;
        if (glfwWindowShouldClose(window)) {
            exit_status = 1;
            exit_loop = true;
        }
        n++;
        glfwSwapBuffers(window);
    };

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
    #else
    while (!exit_loop) loop();
    #endif
    return exit_status;
}
