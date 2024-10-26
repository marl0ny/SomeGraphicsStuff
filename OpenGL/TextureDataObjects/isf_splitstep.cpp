/*
Incompressible Schrodinger Flow (ISF) implementation.

The ISF originates from these two papers:

 - Chern, A., Knöppel, F., Pinkall, U., Schröder, P., Weißmann, S. (2016).
   Schrödinger's smoke. ACM Transactions on Graphics, 35(4), 1-13.
   https://doi.org/10.1145/2897824.2925868

 - Chern, A. (2017). Fluid Dynamics with Incompressible Schrödinger Flow.
   Dissertation.
   https://cseweb.ucsd.edu/~alchern/projects/PhDThesis/thesis_reduced.pdf

*/
#include "isf_splitstep.hpp"

// #include <OpenGL/OpenGL.h>
#include <cstdlib>
#include <new>
#define GL_SILENCE_DEPRECATION

#include "texture_data.hpp"
#include "draw_texture_data.hpp"
#include "interactor.hpp"
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


int isf_splitstep(Renderer *renderer) {

    GLFWwindow *window = renderer->window;
    int main_frame = renderer->main_frame;

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
         = DrawTexture2DData(Path("./shaders/fluids/isf/init.frag"));
    auto init_dist_command
         = DrawTexture2DData(Path("./shaders/fluids/init-dist.frag"));

    auto set_initial_blob_params = [&](
        float rx, float ry,
        float current_amplitude, float current_radius, int nx, int ny,
        float blob_amplitude, float blob_size
    ){
        init_current_command.set_float_uniforms(
            {{"amplitude", current_amplitude},
            {"radius", current_radius}});
        init_current_command.set_vec2_uniforms(
            {{"r0", {.x=rx, .y=ry}}});
        init_current_command.set_ivec2_uniforms(
            {{"wavenumber", {.x=nx, .y=ny}}});
        init_dist_command.set_float_uniforms(
            {{"amplitude", blob_amplitude},{"sigma", blob_size}});
        init_dist_command.set_vec2_uniforms({{"r0", 
            {.x=rx, .y=ry}}});
    };

    set_initial_blob_params(0.25, 0.25, 1.0, 0.1, 45, 45, 3.0, 0.1);
    auto dist0= init_dist_command.create(HALF_FLOAT, NX, NY, true,
                                         GL_REPEAT, GL_REPEAT,
                                         GL_LINEAR, GL_LINEAR);
    auto tmp0 = init_current_command.create(COMPLEX, NX, NY, true,
                                            GL_REPEAT, GL_REPEAT,
                                            GL_LINEAR, GL_LINEAR);

    set_initial_blob_params(0.75, 0.75, 1.0, 0.1, -40, -40, -3.0, 0.1);   
    auto dist1 = init_dist_command.create(HALF_FLOAT, NX, NY, true,
                                          GL_REPEAT, GL_REPEAT,
                                          GL_LINEAR, GL_LINEAR);
    auto tmp1 = init_current_command.create(COMPLEX, NX, NY, true,
                                            GL_REPEAT, GL_REPEAT,
                                            GL_LINEAR, GL_LINEAR);
    auto advect_command
        = DrawTexture2DData(Path("./shaders/fluids/bw-advect.frag"));
    advect_command.set_float_uniforms({{"width", width}, {"height", height},
                                       {"dt", dt}});
    auto advect_higher_or_com
        = DrawTexture2DData(Path("./shaders/fluids/advect-higher-or.frag"));
    advect_higher_or_com.set_float_uniforms({{"width", width},
                                             {"height", height},
                                             {"dx", dx}, {"dy", dy},
                                             {"dt", dt}});

    auto dist = dist0 + dist1;
    auto psi_u = 0.999*tmp0*tmp1;
    auto psi_d = sqrt(1.0 - conj(psi_u)*psi_u);
    auto pressure = 0.0*psi_u;
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
        = DrawTexture2DData(Path("./shaders/fluids/poisson-jacobi.frag"));

    auto poisson_func_iterative = [&](Texture2DData &rho,
                                      Texture2DData &x0,
                                      int n_iterations) -> Texture2DData {
        laplacian_solve_command.set_float_uniforms({{"dx", dx}, {"dy", dy},
                                                    {"width", width},
                                                    {"height", height}});
        auto x = x0;
        for (int i = 0; i < n_iterations; i++)
            laplacian_solve_command.draw(x, "prevIterTex", x, "bTex", rho);
        return x;
    };

    auto h_psi_func = [=](Texture2DData &psi) -> Texture2DData {
        return funcs2D::ifft(p_propagator*funcs2D::fft(psi));
    };

    auto current_func = [=](const Texture2DData &psi_u,
                                      const Texture2DData &psi_d
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

    auto div_j_command
        = DrawTexture2DData(Path("./shaders/fluids/isf/div-j.frag"));

    auto pressure_func = [=](std::vector<Texture2DData> &j,
                              Texture2DData &pressure) -> Texture2DData {
        Grad2DParams grad_params {.dx=dx, .dy=dy,
            .width=width, .height=height,
            .staggered=-1, .order_of_accuracy=4};
        auto grad_dot_j = funcs2D::ddx(j[0], grad_params) 
            + funcs2D::ddy(j[1], grad_params);
        auto pressure_f = pressure.cast_to(FLOAT, X);
        return poisson_func_iterative(grad_dot_j, pressure_f, 10
                                      ).cast_to(COMPLEX, X, NONE);
    };

    auto step = [&](
        Texture2DData &dist,
        Texture2DData &psi_u, Texture2DData &psi_d
        ) -> std::vector<Texture2DData> {
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
        pressure = pressure_func(current, pressure);
        auto exp_ip_hbar = exp(-(imag_unit/hbar)*pressure);
        psi_u = exp_ip_hbar*psi_u;
        psi_d = exp_ip_hbar*psi_d;
        return {psi_u, psi_d};
    };

    auto modify_current_command
        = DrawTexture2DData(
            Path("./shaders/fluids/isf/modify-current.frag"));
            
    auto modify_current_func = [&](
        Texture2DData &dst, Texture2DData &src,
        float radius, float rx, float ry, int nx, int ny
    ) {
        modify_current_command.set_float_uniforms(
            {{"radius", 0.1}}
        );
        modify_current_command.set_vec2_uniforms(
            {{"r0", {.x=rx, .y=ry}}}
        );
        modify_current_command.set_ivec2_uniforms(
            {{"wavenumber", {.x=nx, .y=ny}}}
        );
        modify_current_command.draw(
            dst, "waveFuncTex", src);
    };

    auto display_main_view = [&](Texture2DData &dist) {
        bind_quad(main_frame, view_program);
        dist.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
    };

    int k = 0;
    bool exit_loop = false;
    // int steps_per_frame = 1;
    Interactor interactor(window);
    auto dist_display 
        = Texture2DData(HALF_FLOAT, NX, NY, 
        true, GL_REPEAT, GL_REPEAT,
         GL_LINEAR, GL_LINEAR);
    float amplitude_factor = 2.0;
    loop = [&] {
        glfwPollEvents();
        interactor.click_update(renderer);
        DVec2 left_click = interactor.get_mouse_position();
        glViewport(0, 0, NX, NY);
        if (interactor.left_pressed()) {
            double rx = left_click.x;
            double ry = left_click.y;
            init_dist_command.set_float_uniforms(
                {{"amplitude", amplitude_factor}, 
                {"sigma", 0.1}});
            init_dist_command.set_vec2_uniforms({{"r0", 
                {.x=(float)rx, .y=(float)ry}}});
            auto dist_tmp = init_dist_command.create(HALF_FLOAT,
                NX, NY, true,
                GL_REPEAT, GL_REPEAT,
                GL_LINEAR, GL_LINEAR) + dist;
            dist_display = dist_tmp;
        }
        else if (interactor.left_released()) {
            double rx = left_click.x;
            double ry = left_click.y;
            double vx = interactor.get_mouse_delta().ind[0];
            double vy = interactor.get_mouse_delta().ind[1];
            int wnx = (int)std::max(-width/4.0, 
                    std::min(width/4.0, 8.0*width*vx));
            int wny = (int)std::max(-height/4.0, std::
                        min(height/4.0, 8.0*height*vy));
            auto psi_tmp = psi_u;
            modify_current_func(psi_u, psi_tmp, 0.05, rx, ry, wnx, wny);
            init_dist_command.set_float_uniforms(
                {{"amplitude", amplitude_factor}, 
                {"sigma", 0.1}});
            init_dist_command.set_vec2_uniforms({{"r0", 
                {.x=(float)rx, .y=(float)ry}}});
            auto dist_tmp = init_dist_command.create(HALF_FLOAT,
                NX, NY, true,
                GL_REPEAT, GL_REPEAT,
                GL_LINEAR, GL_LINEAR) + dist;
            dist = dist_tmp;
            dist_display = dist_tmp;
            amplitude_factor *= -1.0;
        } else {
            auto psi_ud = step(dist, psi_u, psi_d);
            psi_u = psi_ud[0];
            psi_d = psi_ud[1];
            dist_display = dist;
        }
        glViewport(0, 0, window_width, window_height);
        display_main_view(dist_display);
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
