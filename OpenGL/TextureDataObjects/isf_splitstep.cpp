#include "isf_splitstep.hpp"

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

static const double pi = 3.141592653589793;

/*
Incompressible Schrodinger Flow implementation.

References:

 - Chern, A., Knöppel, F., Pinkall, U., Schröder, P., Weißmann, S. (2016).
   Schrödinger's smoke. ACM Transactions on Graphics, 35(4), 1-13.
   https://doi.org/10.1145/2897824.2925868

 - Chern, A. (2017). Fluid Dynamics with Incompressible Schrödinger Flow.
   Dissertation.
   https://cseweb.ucsd.edu/~alchern/projects/PhDThesis/thesis_reduced.pdf

*/
int isf_splitstep(GLFWwindow *window, frame_id main_frame) {
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    int NX = 256, NY = 256;
    double dt = 2.0;
    double width = (double)NX, height = (double)NY;
    double dx = width/(double)NX, dy = height/(double)NY;
    double hbar = 1.0;
    double m = 1.0;
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    auto init_current_command
         = DrawTexture2DData(Path("./shaders/isf-init.frag"));
    auto init_dist_command
         = DrawTexture2DData(Path("./shaders/isf-init-dist.frag"));
    init_current_command.set_float_uniforms({{"amplitude", 1.0},
                                             {"radius", 0.1}});
    init_current_command.set_vec2_uniforms({{"r0", {.x=0.25, .y=0.25}}});
    init_current_command.set_ivec2_uniforms({{"wavenumber", {.x=45, .y=45}}});
    init_dist_command.set_float_uniforms({{"amplitude", 3.0},
                                          {"sigma", 0.1}});
    init_dist_command.set_vec2_uniforms({{"r0", {.x=0.25, .y=0.25}}});
    auto dist0= init_dist_command.create(HALF_FLOAT, NX, NY, true,
                                         GL_REPEAT, GL_REPEAT,
                                         GL_LINEAR, GL_LINEAR);
    auto tmp0 = init_current_command.create(COMPLEX, NX, NY, true,
                                            GL_REPEAT, GL_REPEAT,
                                            GL_LINEAR, GL_LINEAR);
    init_dist_command.set_float_uniforms({{"amplitude", -3.0}});
    init_current_command.set_vec2_uniforms({{"r0", {.x=0.75, .y=0.75}}});
    init_dist_command.set_vec2_uniforms({{"r0", {.x=0.75, .y=0.75}}});
    init_current_command.set_ivec2_uniforms({{"wavenumber",
                                             {.x=-40, .y=-40}}});
    auto dist1 = init_dist_command.create(HALF_FLOAT, NX, NY, true,
                                          GL_REPEAT, GL_REPEAT,
                                          GL_LINEAR, GL_LINEAR);
    auto tmp1 = init_current_command.create(COMPLEX, NX, NY, true,
                                            GL_REPEAT, GL_REPEAT,
                                            GL_LINEAR, GL_LINEAR);
    auto advect_command
        = DrawTexture2DData(Path("./shaders/isf-bw-advect.frag"));
    advect_command.set_float_uniforms({{"width", width}, {"height", height},
                                       {"dt", dt}});
    auto advect_higher_or_com
        = DrawTexture2DData(Path("./shaders/advect-higher-or.frag"));
    advect_higher_or_com.set_float_uniforms({{"width", width},
                                             {"height", height},
                                             {"dx", dx}, {"dy", dy},
                                             {"dt", dt}});
    auto dist = dist0 + dist1;
    auto psi_u = 0.999*tmp0*tmp1;
    auto psi_d = sqrt(1.0 - conj(psi_u)*psi_u);
    auto imag_unit = std::complex<double>(0.0, 1.0);
    auto x = funcs2D::make_x(-0.5, 0.5, 
        FLOAT, NX, NY) - 0.5/(double)NX;
    auto y = funcs2D::make_y(-0.5, 0.5, 
        FLOAT, NX, NY) - 0.5/(double)NY;
    auto px = funcs2D::fftshift(2.0*pi*x);
    auto py = funcs2D::fftshift(2.0*pi*y);
    auto p_propagator = exp((-imag_unit*dt/(2.0*m*hbar))*
                            (px*px + py*py).cast_to(COMPLEX, X, NONE));
    auto sx = sin(pi*x), sy = sin(pi*y);
    // auto laplace_eigval
    //     = max(((4.0/(dx*dx))*sx*sx + (4.0/(dy*dy))*sy*sy), 0.01);
    auto laplace_eigval = max(px*px + py*py, 0.001);
    laplace_eigval = funcs2D::fftshift(
        laplace_eigval).cast_to(COMPLEX, X, NONE);
    auto poisson_func_fft = [&](Texture2DData &rho) ->Texture2DData {
        return -1.0*funcs2D::ifft(funcs2D::fft(rho/(laplace_eigval)));
    };
    auto laplacian_solve_command
        = DrawTexture2DData(Path("./shaders/poisson-jacobi.frag"));

    auto poisson_func_iterative = [&](Texture2DData &rho,
                                      Texture2DData &x0,
                                      int n_iterations) -> Texture2DData {
        laplacian_solve_command.set_float_uniforms({{"dx", dx}, {"dy", dy},
                                                    {"width", width},
                                                    {"height", height}});
        auto x = x0;
        for (int i = 0; i < n_iterations; i++) {
            laplacian_solve_command.draw(x, "prevIterTex", x, "bTex", rho);
        }
        return x;
    };
    auto h_psi_func = [=](Texture2DData &psi) -> Texture2DData {
        return funcs2D::ifft(p_propagator*funcs2D::fft(psi));
    };
    auto current_func = [=](Texture2DData &psi_u,
                            Texture2DData &psi_d
                            ) -> std::vector<Texture2DData> {
        Grad2DParams grad_params {.dx=dx, .dy=dy,
            .width=width, .height=height,
            .staggered=1, .order_of_accuracy=4};
        auto hbar_i = -imag_unit*hbar;
        auto psi_u_dag 
            = funcs2D::roll(conj(psi_u), -dx/(2.0*width), -dy/(2.0*height));
        auto psi_d_dag 
            = funcs2D::roll(conj(psi_d), -dx/(2.0*width), -dy/(2.0*height));
        auto jx = (hbar_i*(psi_u_dag*funcs2D::ddx(psi_u, grad_params)
                           + psi_d_dag*funcs2D::ddx(psi_d, grad_params))
                   ).cast_to(COMPLEX, X, NONE);
        auto jy = (hbar_i*(psi_u_dag*funcs2D::ddy(psi_u, grad_params)
                           + psi_d_dag*funcs2D::ddy(psi_d, grad_params))
                   ).cast_to(COMPLEX, X, NONE);
        return std::vector<Texture2DData> {jx, jy};
    };
    auto pressure_func = [=](std::vector<Texture2DData> &j) -> Texture2DData {
        Grad2DParams grad_params {.dx=dx, .dy=dy,
            .width=width, .height=height,
            .staggered=-1, .order_of_accuracy=4};
        auto grad_dot_j = funcs2D::ddx(j[0], grad_params)
             + funcs2D::ddy(j[1], grad_params);
        return poisson_func_fft(grad_dot_j);
    };
    auto div_j_command
        = DrawTexture2DData(Path("./shaders/isf-div-j.frag"));
    auto pressure_func2 = [&](Texture2DData &psi_u,
                              Texture2DData &psi_d,
                              Texture2DData &pressure) -> Texture2DData {
        div_j_command.set_float_uniforms({{"dx", dx}, {"dy", dy},
          {"width", width}, {"height", height}});
        auto div_j = funcs2D::zeroes(FLOAT, NX, NY);
        div_j_command.draw(div_j, "texU", psi_u, "texD", psi_d);
        auto tmp = div_j.cast_to(COMPLEX, X, NONE);
        return poisson_func_fft(tmp);
    };
    auto pressure_func3 = [&](Texture2DData &psi_u,
                              Texture2DData &psi_d,
                              Texture2DData &pressure) -> Texture2DData {
        div_j_command.set_float_uniforms({{"dx", dx}, {"dy", dy},
                                          {"width", width}, {"height", height}});
        auto div_j = funcs2D::zeroes(FLOAT, NX, NY);
        div_j_command.draw(div_j, "texU", psi_u, "texD", psi_d);
        auto pressuref = pressure.cast_to(FLOAT, X);
        return poisson_func_iterative(div_j, pressuref, 20
                                      ).cast_to(COMPLEX, X, NONE);
    };
    auto pressure_func4 = [=](std::vector<Texture2DData> &j,
                              Texture2DData &pressure) -> Texture2DData {
        Grad2DParams grad_params {.dx=dx, .dy=dy,
            .width=width, .height=height,
            .staggered=-1, .order_of_accuracy=4};
        auto grad_dot_j = funcs2D::ddx(j[0], grad_params) 
            + funcs2D::ddy(j[1], grad_params);
        auto pressuref = pressure.cast_to(FLOAT, X);
        return poisson_func_iterative(grad_dot_j, pressuref, 10
                                      ).cast_to(COMPLEX, X, NONE);
    };

    auto pressure = 0.0*psi_u;

    int k = 0;
    bool exit_loop = false;
    loop = [&] {
         glViewport(0, 0, NX, NY);
        psi_u = h_psi_func(psi_u);
        psi_d = h_psi_func(psi_d);
        auto norm = sqrt(psi_u*conj(psi_u) + psi_d*conj(psi_d));
        psi_u = psi_u / norm;
        psi_d = psi_d / norm;
        auto current = current_func(psi_u, psi_d);
        auto v = cast_to(FLOAT2,
                               current[0], X, NONE, NONE, NONE,
                               current[1], NONE, X, NONE, NONE);
        advect_command.set_float_uniforms({{"dt", dt}});
        auto forward 
            = funcs2D::zeroes(FLOAT, NX, NY);
        advect_command.draw(forward, "velocityTex", v, "densityTex", dist);
        advect_command.set_float_uniforms({{"dt", -dt}});
        auto backward 
            = funcs2D::zeroes(FLOAT, NX, NY);
        advect_command.draw(backward,
                            "velocityTex", v, "densityTex", forward);
        advect_higher_or_com.draw(dist, "tex", dist,
                                  "velocityTex", v,
                                  "advectForwardTex", forward,
                                  "advectReverseTex", backward);
        // dist = forward + 0.5*(dist - backward);

        // auto pressure = pressure_func(current);
        // pressure
        // = pressure_func2(psi_u, psi_d, pressure).cast_to(COMPLEX, X, NONE);
        pressure = pressure_func4(current, pressure);
        auto exp_ip_hbar = exp(-(imag_unit/hbar)*pressure);
        psi_u = exp_ip_hbar*psi_u;
        psi_d = exp_ip_hbar*psi_d;
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        dist.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
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
