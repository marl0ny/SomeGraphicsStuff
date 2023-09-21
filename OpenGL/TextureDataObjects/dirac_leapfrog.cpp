#include "dirac_leapfrog.hpp"

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

/* Numerically solving the Dirac equation using centred second order
finite differences in time, where the upper and lower spinors can be optionally
staggered in space.

References:
 - Wikipedia contributors. (2021, June 16).
   Dirac equation. In Wikipedia, The Free Encyclopedia.
 - Wikipedia contributors. (2021, August 5).
   Dirac spinor. In Wikipedia, The Free Encyclopedia.
 - Hammer, R., Pötz W. (2014).
   Staggered grid leap-frog scheme for the (2 + 1)D Dirac equation.
   Computer Physics Communications, 185(1), 40-53.
   https://doi.org/10.1016/j.cpc.2013.08.013

*/
int dirac_leapfrog(GLFWwindow *window, frame_id main_frame) {
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    int NX = 256;
    int NY = 256;
    double hbar = 1.0;
    double m = 1.0;
    double c = 137.036;
    double dt = 0.000028;
    double width = 2.0;
    double height = 2.0;
    // double pi = 3.141592653589793;
    double dx = width/(float)NX, dy = height/(float)NY;
    auto imag_unit = std::complex<double>(0.0, 1.0);
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    auto x = make_x(-0.5, 0.5, FLOAT, NX, NY);
    auto y = make_y(-0.5, 0.5, FLOAT, NX, NY);
    auto command
        = DrawTexture2DData(Path("./shaders/init-gaussian.frag"));
    command.set_float_uniforms({{"amplitude", 5.0},
                                {"sigmaX", 0.05}, {"sigmaY", 0.05}});
    command.set_vec2_uniforms({{"r0", {.x=0.25, .y=0.25}}, });
    command.set_ivec2_uniforms({{"wavenumber", {.x=0, .y=0}}, });
    auto phi0 = zeroes(COMPLEX2, NX, NY);
    command.draw(phi0);
    std::vector<Texture2DData> psi0 {phi0, 0.0*phi0};
    std::vector<Texture2DData> psi1 {phi0, 0.0*phi0};
    std::vector<Texture2DData> psi2 {phi0, 0.0*phi0};
    std::vector<Texture2DData> vec_potential {
        0.0*10000.0*(x*x + y*y).cast_to(COMPLEX2, X, NONE, X, NONE),
        15000.0*y.cast_to(COMPLEX, X, NONE, NONE, NONE),
        -15000.0*x.cast_to(COMPLEX, X, NONE, NONE, NONE),
        0.0*phi0.cast_to(COMPLEX, X, NONE)
    };
    auto sigma_dot_vec_potential
        = [=](Texture2DData &psi, std::vector<Texture2DData> &vec_potential,
              int stag) -> Texture2DData {
        Texture2DData psi0 = psi.cast_to(COMPLEX, Channel::X, Channel::Y);
        Texture2DData psi1 = psi.cast_to(COMPLEX, Channel::Z, Channel::W);
        struct Vec2 tmp {.x=-(float)(dx/(2.0*width)),
            .y=-(float)(dy/(2.0*height))};
        auto vx = (stag == 1)? roll(vec_potential[1], tmp): vec_potential[1];
        auto vy = (stag == 1)? roll(vec_potential[2], tmp): vec_potential[2];
        auto vz = (stag == 1)? roll(vec_potential[3], tmp): vec_potential[3];
        Texture2DData tmp0 = (vz*psi0 + (vx - imag_unit*vy)*psi1);
        Texture2DData tmp1 = ((vx + imag_unit*vy)*psi0 - vz*psi1);
        return cast_to(COMPLEX2,
                       tmp0, X, Y, NONE, NONE, tmp1, NONE, NONE, X, Y);
    };
    auto sigma_dot_grad = [=](Texture2DData &psi, int stag) -> Texture2DData {
        auto psi0 = psi.cast_to(COMPLEX, Channel::X, Channel::Y);
        auto psi1 = psi.cast_to(COMPLEX, Channel::Z, Channel::W);
        struct Grad2DParams grad_params {.dx=dx, .dy=dy,
            .width=width, .height=height,
            .staggered=stag, .order_of_accuracy=4};
        auto tmp0 = (ddx(psi1, grad_params)
                     - imag_unit*ddy(psi1, grad_params));
        auto tmp1 = (ddx(psi0, grad_params)
                     + imag_unit*ddy(psi0, grad_params));
        return cast_to(COMPLEX2,
                       tmp0, X, Y, NONE, NONE, tmp1, NONE, NONE, X, Y);
    };
    auto eom = [=](std::vector<Texture2DData> &psi2,
                   std::vector<Texture2DData> &psi0,
                   std::vector<Texture2DData> &psi1,
                   std::vector<Texture2DData> &vec_potential,
                   double dt) {
        auto idtmc2_2hbar = dt*imag_unit*m*c*c/(2.0*hbar);
        auto idtqphi_2hbar = (imag_unit*dt/(2.0*hbar))*vec_potential[0];
        auto iqdt_hbar = imag_unit*dt/hbar;
        psi2[0] = ((-dt*c)*sigma_dot_grad(psi1[1], 0)
                   + iqdt_hbar*sigma_dot_vec_potential(psi1[1],
                                                       vec_potential, 0)
                   + (1.0 - idtmc2_2hbar - idtqphi_2hbar)*psi0[0]
                  )/(1.0 + idtmc2_2hbar + idtqphi_2hbar);
        // idtqphi_2hbar
        //     = roll(idtqphi_2hbar, -dx/(2.0*width), -dy/(2.0*height));
        psi2[1] = ((-dt*c)*sigma_dot_grad(psi1[0], 0)
                   + iqdt_hbar*sigma_dot_vec_potential(psi1[0],
                                                       vec_potential, 0)
                   + (1.0 + idtmc2_2hbar - idtqphi_2hbar)*psi0[1]
                  )/(1.0 - idtmc2_2hbar + idtqphi_2hbar);
    };
    eom(psi1, psi0, psi0, vec_potential, 0.5*dt);

    int k = 0;
    bool exit_loop = false;
    loop = [&] {
        glViewport(0, 0, NX, NY);
        for (int i = 0; i < 10; i++) {
            eom(psi2, psi0, psi1, vec_potential, dt);
            swap(psi2[0], psi1[0]);
            swap(psi0[0], psi2[0]);
            swap(psi2[1], psi1[1]);
            swap(psi0[1], psi2[1]);

        }
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        psi0[0].set_as_sampler2D_uniform("tex");
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
    };

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
    #else
    while(!exit_loop)
        loop();
    #endif
    return exit_status;
}
 