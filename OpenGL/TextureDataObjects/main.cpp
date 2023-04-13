#include <GLFW/glfw3.h>
#include "texture_data.hpp"
#include "draw_texture_data.hpp"
#include <OpenGL/OpenGL.h>
#include <vector>
#include "fft.h"

#define GL_SILENCE_DEPRECATION

void complex_func(GLFWwindow *window,
                  int window_width, int window_height) {
    int NX = window_width;
    int NY = window_height;
    std::complex<double> imag_unit {0.0, 1.0};
    frame_id main_frame = new_quad(NULL);
    int view_program = make_quad_program("./shaders/view.frag");
    auto x_float = make_x(-0.5, 0.5, FLOAT, NX, NY);
    auto y_float = make_y(-0.5, 0.5, FLOAT, NX, NY);
    auto x = x_float.cast_to(COMPLEX, X, NONE);
    auto y = y_float.cast_to(COMPLEX, X, NONE);
    auto z = x + imag_unit*y;
    auto f = sin(3.14159*z);
    // auto f = 10.0*(z - 0.25)*(z + 0.25)*z;
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        bind_quad(main_frame, view_program);
        f.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        // (psi1 / 3.0).paste_to_quad(main_frame);
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}

void poisson(GLFWwindow *window, int window_width, int window_height) {
    int NX = 128, NY = 128;
    // double width = (double)NX, height = (double)NY;
    // double dx = width/(double)NX, dy = height/(double)NY;
    frame_id main_frame = new_quad(NULL);
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    auto x0 = make_x(0.0, 1.0, FLOAT, NX, NY) - 0.5 - 0.5/(double)NX;
    auto y0 = make_y(0.0, 1.0, FLOAT, NX, NY) - 0.5 - 0.5/(double)NY;
    // auto x = max(x0, 0.5/(double)NX).cast_to(COMPLEX, X, NONE);
    // auto y = max(y0, 0.5/(double)NY).cast_to(COMPLEX, X, NONE);
    auto x = x0; // .cast_to(COMPLEX, X, NONE);
    auto y = y0; // .cast_to(COMPLEX, X, NONE);
    double pi = 3.141592653589793;
    auto px = fftshift(2.0*pi*x);
    auto py = fftshift(2.0*pi*y);
    auto w = 0.25*exp(-0.5*(x*x + y*y)/(0.01*0.01)).cast_to(COMPLEX, X, NONE);
    w = w + 0.5*exp((-0.5/(0.01*0.01))*((x-0.3)*(x-0.3)
                                         + (y+0.1)*(y+0.1))
                    ).cast_to(COMPLEX, X, NONE);
    /*w = w + 10.0*exp((-0.5/(0.01*0.01))*((x+0.3)*(x+0.3)
                                         + (y-0.3)*(y-0.3)));
    w = w + 10.0*exp((-0.5/(0.01*0.01))*((x+0.1)*(x+0.1)
                                         + (y+0.3)*(y+0.3)));*/
    // del^2 u = rho
    // -del^2 u = -rho
    // p^2 u = -rho
    // u = -rho/p^2
    auto sx = sin(x0*pi);
    auto sy = sin(y0*pi);
    // auto eigval = min(((-4.0/(dx*dx))*sx*sx + (-4.0/(dy*dy))*sy*sy), -0.001);
    // eigval = fftshift(eigval).cast_to(COMPLEX, X, NONE);
    auto eigval = max(px*px + py*py, 0.001).cast_to(COMPLEX, X, NONE);
    auto phi = -1.0*ifft(fft(w)/eigval);
    phi = phi.cast_to(COMPLEX, X, Y, NONE, NONE);
    // auto laplacian_phi = 100.0*phi.laplacian({.dx=dx, .dy=dy,
    //         .width=width, .height=height, .order_of_accuracy=4});
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        phi.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

}

/* Finite difference waves simulation using
second order centred difference in time.*/
void waves(GLFWwindow *window,
           int window_width, int window_height) {
    int NX = window_width, NY = window_height;
    frame_id main_frame = new_quad(NULL);
    int view_program = make_quad_program("./shaders/view.frag");
    Texture2DData x = make_x(-0.5, 0.5, FLOAT, NX, NY);
    Texture2DData y = make_y(-0.5, 0.5, FLOAT, NX, NY);
    auto w1 = 10.0*exp(-0.5*(x*x + y*y)/(0.01*0.01));
    w1 = w1 + 10.0*exp((-0.5/(0.01*0.01))*((x-0.3)*(x-0.3) + (y+0.1)*(y+0.1)));
    w1 = w1 + 10.0*exp((-0.5/(0.01*0.01))*((x+0.3)*(x+0.3) + (y-0.3)*(y-0.3)));
    w1 = w1 + 10.0*exp((-0.5/(0.01*0.01))*((x+0.1)*(x+0.1) + (y+0.3)*(y+0.3)));
    // w1 = w1.cast_to(COMPLEX, Channel::NONE, Channel::X);
    auto w2 = w1;
    auto w3 = w2;
    auto step = [=](Texture2DData &w0, Texture2DData &w1,
                    double dt) -> Texture2DData {
        auto laplacian_w1 = w1.laplacian({.dx=1.0, .dy=1.0,
                .width=(double)NX, .height=(double)NY,
                .order_of_accuracy=8});
        return dt*laplacian_w1 + 2.0*w1 - w0;
    };
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        for (int j = 0; j < 10; j++) {
            if (k == 0 && j == 0) {
                w2 = step(w1, w1, 0.05);
            } else {
                w3 = step(w1, w2, 0.1);
                swap(w3, w2);
                swap(w1, w3);
            }
        }
        bind_quad(main_frame, view_program);
        w1.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}

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
void dirac_leapfrog(GLFWwindow *window,
                    int window_width, int window_height) {
    int NX = 256;
    int NY = 256;
    frame_id main_frame = new_quad(NULL);
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
    command.set_float_uniforms({{"amplitude", 5.0}, {"sigma", 0.05}});
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
        struct Vec2 tmp {.x=-(float)(dx/(2.0*width)), .y=-(float)(dy/(2.0*height))};
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
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
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
        glfwSwapBuffers(window);
    }

}

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
void stable_fluids(GLFWwindow *window, int window_width, int window_height) {
    int NX = 256, NY = 256;
    frame_id main_frame = new_quad(NULL);
    double dt = 1.0;
    double width = (double)NX, height = (double)NY;
    double dx = width/(double)NX, dy = height/(double)NY;
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    auto advect_com = DrawTexture2DData(Path("./shaders/isf-bw-advect.frag"));
    advect_com.set_float_uniforms({{"width", width}, {"height", height}});
    auto advect_higher_or_com
        = DrawTexture2DData(Path("./shaders/advect-higher-or.frag"));
    advect_higher_or_com.set_float_uniforms({{"width", width}, {"height", height},
                                             {"dx", dx}, {"dy", dy}, {"dt", dt}});
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
            .order_of_accuracy=4, .staggered=1
        };
        auto div_dot_v = ddx(vel, grad_params).cast_to(FLOAT, X)
                                + ddy(vel, grad_params).cast_to(FLOAT, Y);
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
            .order_of_accuracy=4, .staggered=-1
        };
        auto dpdx = ddx(p, grad_params);
        auto dpdy = ddy(p, grad_params);
        auto grad_p = dpdx.cast_to(FLOAT2, X, NONE)
                                     + dpdy.cast_to(FLOAT2, NONE, X);
        return v - grad_p;
    };
    auto pressure = 0.0*dist;
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        glViewport(0, 0, NX, NY);
        dist = advect_func(dist, vel);
        vel = advect_func(vel, vel);
        pressure = pressure_func(vel, pressure, 15);
        vel = pressure_project_func(vel, pressure);
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        dist.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}

/* void particles(GLFWwindow *window, int window_width, int window_height) {
    int NX = 256, NY = 256;
    frame_id main_frame = new_quad(NULL);
    double dt = 2.0;
}*/

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
void isf_splitstep(GLFWwindow *window,
                   int window_width, int window_height) {
    int NX = 256, NY = 256;
    frame_id main_frame = new_quad(NULL);
    double dt = 2.0;
    double width = (double)NX, height = (double)NY;
    double dx = width/(double)NX, dy = height/(double)NY;
    double hbar = 1.0;
    double m = 1.0;
    double pi = 3.141592653589793;
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    auto init_current_command
         = DrawTexture2DData(Path("./shaders/isf-init.frag"));
    auto init_dist_command
         = DrawTexture2DData(Path("./shaders/isf-init-dist.frag"));
    init_current_command.set_float_uniforms({{"amplitude", 1.0}, 
                                             {"radius", 0.1}});
    init_current_command.set_vec2_uniforms({{"r0", {.x=0.25, .y=0.25}}});
    init_current_command.set_ivec2_uniforms({{"wavenumber", {.x=35, .y=30}}});
    init_dist_command.set_float_uniforms({{"amplitude", 5.0}, 
                                          {"sigma", 0.1}});
    init_dist_command.set_vec2_uniforms({{"r0", {.x=0.25, .y=0.25}}});
    auto dist0= init_dist_command.create(FLOAT, NX, NY, true,
                                                  GL_REPEAT, GL_REPEAT, 
                                                  GL_LINEAR, GL_LINEAR);
    auto tmp0 = init_current_command.create(COMPLEX, NX, NY, true,
                                                    GL_REPEAT, GL_REPEAT,
                                                GL_LINEAR, GL_LINEAR);
    init_dist_command.set_float_uniforms({{"amplitude", -5.0}});
    init_current_command.set_vec2_uniforms({{"r0", {.x=0.75, .y=0.75}}});
    init_dist_command.set_vec2_uniforms({{"r0", {.x=0.75, .y=0.75}}});
    init_current_command.set_ivec2_uniforms({{"wavenumber", 
                                             {.x=-30, .y=-30}}});
    auto dist1 = init_dist_command.create(FLOAT, NX, NY, true,
                                                  GL_REPEAT, GL_REPEAT, 
                                                  GL_LINEAR, GL_LINEAR);
    auto tmp1 = init_current_command.create(COMPLEX, NX, NY, true,
                                                  GL_REPEAT, GL_REPEAT, 
                                                  GL_LINEAR, GL_LINEAR);
    auto advect_command = DrawTexture2DData(Path("./shaders/isf-bw-advect.frag"));
    advect_command.set_float_uniforms({{"width", width}, {"height", height},
                                       {"dt", dt}});
    auto advect_higher_or_com
        = DrawTexture2DData(Path("./shaders/advect-higher-or.frag"));
    advect_higher_or_com.set_float_uniforms({{"width", width}, {"height", height},
                                             {"dx", dx}, {"dy", dy}, {"dt", dt}});
    // advect_command.set_texture2D_data("", );
    // advect_command.set_texture2D_data("", );
    /* auto command
        = DrawTexture2DData(Path("./shaders/init-gaussian.frag"));
    command.set_float_uniforms({{"amplitude", 1.0}, {"sigma", 0.07}});
    command.set_vec2_uniforms({{"r0", {.x=0.25, .y=0.25}}, });
    command.set_ivec2_uniforms({{"wavenumber", {.x=20, .y=20}}, });
    auto tmp0 = command.create(COMPLEX, NX, NY, true,
                               GL_REPEAT, GL_REPEAT,
                               GL_LINEAR, GL_LINEAR);
    command.set_vec2_uniforms({{"r0", {.x=0.75, .y=0.75}}, });
    command.set_ivec2_uniforms({{"wavenumber", {.x=-20, .y=-20}}, });
    auto tmp1 = command.create(COMPLEX, NX, NY, true,
                               GL_REPEAT, GL_REPEAT,
                               GL_LINEAR, GL_LINEAR);*/
    auto dist = dist0 + dist1;
    auto psi_u = 0.99*tmp0*tmp1;
    auto psi_d = sqrt(1.0 - conj(psi_u)*psi_u);
    auto imag_unit = std::complex<double>(0.0, 1.0);
    auto x = make_x(-0.5, 0.5, FLOAT, NX, NY) - 0.5/(double)NX;
    auto y = make_y(-0.5, 0.5, FLOAT, NX, NY) - 0.5/(double)NY;
    auto px = fftshift(2.0*pi*x);
    auto py = fftshift(2.0*pi*y);
    auto p_propagator = exp((-imag_unit*dt/(2.0*m*hbar))*
                            (px*px + py*py).cast_to(COMPLEX, X, NONE));
    auto sx = sin(pi*x), sy = sin(pi*y);
    // auto laplace_eigval
    //     = max(((4.0/(dx*dx))*sx*sx + (4.0/(dy*dy))*sy*sy), 0.01);
    auto laplace_eigval = max(px*px + py*py, 0.001);
    laplace_eigval = fftshift(laplace_eigval).cast_to(COMPLEX, X, NONE);
    auto poisson_func_fft = [&](Texture2DData &rho) ->Texture2DData {
        return -1.0*ifft(fft(rho/(laplace_eigval)));
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
        return ifft(p_propagator*fft(psi));
    };
    auto current_func = [=](Texture2DData &psi_u,
                            Texture2DData &psi_d

                            ) -> std::vector<Texture2DData> {
        Grad2DParams grad_params {.dx=dx, .dy=dy,
            .width=width, .height=height,
            .order_of_accuracy=4, .staggered=1};
        auto hbar_i = -imag_unit*hbar;
        auto psi_u_dag = roll(conj(psi_u), -dx/(2.0*width), -dy/(2.0*height));
        auto psi_d_dag = roll(conj(psi_d), -dx/(2.0*width), -dy/(2.0*height));
        auto jx = (hbar_i*(psi_u_dag*ddx(psi_u, grad_params)
                           + psi_d_dag*ddx(psi_d, grad_params))
                   ).cast_to(COMPLEX, X, NONE);
        auto jy = (hbar_i*(psi_u_dag*ddy(psi_u, grad_params)
                           + psi_d_dag*ddy(psi_d, grad_params))
                   ).cast_to(COMPLEX, X, NONE);
        return std::vector<Texture2DData> {jx, jy};
    };
    auto pressure_func = [=](std::vector<Texture2DData> &j) -> Texture2DData {
        Grad2DParams grad_params {.dx=dx, .dy=dy,
            .width=width, .height=height,
            .order_of_accuracy=4, .staggered=-1};
        auto grad_dot_j = ddx(j[0], grad_params) + ddy(j[1], grad_params);
        return poisson_func_fft(grad_dot_j);
    };
    auto div_j_command
        = DrawTexture2DData(Path("./shaders/isf-div-j.frag"));
    auto pressure_func2 = [&](Texture2DData &psi_u,
                              Texture2DData &psi_d) -> Texture2DData {
        div_j_command.set_float_uniforms({{"dx", dx}, {"dy", dy},
          {"width", width}, {"height", height}});
        auto div_j = zeroes(FLOAT, NX, NY);
        div_j_command.draw(div_j, "texU", psi_u, "texD", psi_d);
        return poisson_func_fft(div_j);
    };
    auto pressure_func3 = [&](Texture2DData &psi_u,
                              Texture2DData &psi_d,
                              Texture2DData &pressure) -> Texture2DData {
        div_j_command.set_float_uniforms({{"dx", dx}, {"dy", dy},
                                          {"width", width}, {"height", height}});
        auto div_j = zeroes(FLOAT, NX, NY);
        div_j_command.draw(div_j, "texU", psi_u, "texD", psi_d);
        auto pressuref = pressure.cast_to(FLOAT, X);
        return poisson_func_iterative(div_j, pressuref, 15
                                      ).cast_to(COMPLEX, X, NONE);
    };
    auto pressure_func4 = [=](std::vector<Texture2DData> &j,
                              Texture2DData &pressure) -> Texture2DData {
        Grad2DParams grad_params {.dx=dx, .dy=dy,
            .width=width, .height=height,
            .order_of_accuracy=4, .staggered=-1};
        auto grad_dot_j = ddx(j[0], grad_params) + ddy(j[1], grad_params);
        auto pressuref = pressure.cast_to(FLOAT, X);
        return poisson_func_iterative(grad_dot_j, pressuref, 10
                                      ).cast_to(COMPLEX, X, NONE);
    };

    auto pressure = 0.0*psi_u;
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
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
        auto forward = zeroes(FLOAT, NX, NY);
        advect_command.draw(forward, "velocityTex", v, "densityTex", dist);
        advect_command.set_float_uniforms({{"dt", -dt}});
        auto backward = zeroes(FLOAT, NX, NY);
        advect_command.draw(backward, 
                            "velocityTex", v, "densityTex", forward);
        advect_higher_or_com.draw(dist, "tex", dist,
                                  "velocityTex", v, 
                                  "advectForwardTex", forward,
                                  "advectReverseTex", backward);
        // dist = forward + 0.5*(dist - backward);

        // auto pressure = pressure_func(current);
        pressure = pressure_func4(current, pressure);
        auto exp_ip_hbar = exp(-(imag_unit/hbar)*pressure);
        psi_u = exp_ip_hbar*psi_u;
        psi_d = exp_ip_hbar*psi_d;
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        dist.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}

/* Split operator implementation for the Schrodinger equation.

References:

 - James Schloss. The Split-Operator Method. In The Arcane Algorithm Archive.
   https://www.algorithm-archive.org/contents/
   split-operator_method/split-operator_method.html.

 - Wikipedia contributors. (2021, May 6). Split-step method.
   In Wikipedia, The Free Encyclopedia.
   https://en.wikipedia.org/wiki/Split-step_method.

*/
void schrod_splitstep(GLFWwindow *window,
                      int window_width, int window_height) {
    int NX = 256, NY = 256;
    auto imag_unit = std::complex<double>(0.0, 1.0);
    frame_id main_frame = new_quad(NULL);
    double dt = 3.0;
    // std::complex<double> dt = 3.0 - 0.5*imag_unit;
    double hbar = 1.0;
    double m = 1.0;
    // double dx = width/(float)NX, dy = height/(float)NY;
    double pi = 3.141592653589793;
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    auto command
        = DrawTexture2DData(Path("./shaders/init-gaussian.frag"));
    command.set_float_uniforms({{"amplitude", 5.0}, {"sigma", 0.05}});
    command.set_vec2_uniforms({{"r0", {.x=0.25, .y=0.25}}, });
    command.set_ivec2_uniforms({{"wavenumber", {.x=-5, .y=5}}, });
    auto psi1 = command.create(COMPLEX, NX, NY, true,
                               GL_REPEAT, GL_REPEAT,
                               GL_LINEAR, GL_LINEAR);
    auto psi2 = psi1;
    auto x = make_x(-0.5, 0.5, FLOAT, NX, NY) - 0.5/(double)NX;
    auto y = make_y(-0.5, 0.5, FLOAT, NX, NY) - 0.5/(double)NY;
    auto px = fftshift(2.0*pi*x).cast_to(COMPLEX, X, NONE);
    auto py = fftshift(2.0*pi*y).cast_to(COMPLEX, X, NONE);
    auto pot = (0.6*(x*x + y*y)).cast_to(COMPLEX, X, NONE);
    auto p_propagator = exp((-imag_unit*dt/(2.0*m*hbar))*(px*px + py*py));
    auto x_propagator = exp((-imag_unit*(dt/2.0)/hbar)*pot);
    auto h_psi_func = [=](Texture2DData &psi,
                          Texture2DData &pot) -> Texture2DData {
        return x_propagator*ifft(p_propagator*fft(x_propagator*psi));
    };
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        glViewport(0, 0, NX, NY);
        for (int i = 0; i < 3; i++) {
            psi2 = h_psi_func(psi1, pot);
            swap(psi1, psi2);
        }
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        psi1.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

}

/* Numerically solving the Schrodinger equation using explicit finite
differences. This follows an article by Visscher.

Reference:

Visscher, P. (1991).
A fast explicit algorithm for the time‐dependent Schrödinger equation.
Computers in Physics, 5<, 596-598.
https://doi.org/10.1063/1.168415

*/
void schrod_leapfrog(GLFWwindow *window,
                     int window_width, int window_height) {
    int NX = 512;
    int NY = 512;
    frame_id main_frame = new_quad(NULL);
    double dt = 0.011;
    double hbar = 1.0;
    double m = 1.0;
    double width = ((double)NX/512.0)*91.0;
    double height = ((double)NX/512.0)*91.0;
    double dx = width/(float)NX, dy = height/(float)NY;
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    auto command
        = DrawTexture2DData(Path("./shaders/init-gaussian.frag"));
    command.set_float_uniforms({{"amplitude", 5.0}, {"sigma", 0.05}});
    command.set_vec2_uniforms({{"r0", {.x=0.2, .y=0.2}}, });
    command.set_ivec2_uniforms({{"wavenumber", {.x=-5, .y=5}}, });
    auto psi1 = command.create(COMPLEX, NX, NY, true,
                               GL_REPEAT, GL_REPEAT,
                               GL_LINEAR, GL_LINEAR);
    auto imag_unit = std::complex<double>(0.0, 1.0);
    auto x = make_x(-0.5, 0.5, FLOAT, NX, NY);
    auto y = make_y(-0.5, 0.5, FLOAT, NX, NY);
    auto pot = (20.0*(x*x + y*y)).cast_to(COMPLEX, X, NONE);
    auto h_psi_func = [=](Texture2DData &psi,
                          Texture2DData &pot) -> Texture2DData {
        auto laplacian_psi = psi.laplacian({.dx=dx, .dy=dy,
                .width=width, .height=height, .order_of_accuracy=4});
        return ((-hbar*hbar)/(2.0*m))*laplacian_psi + pot*psi;
    };
    auto psi2 = psi1 - (imag_unit*(0.5*dt/hbar))*h_psi_func(psi1, pot);
    auto psi3 = psi2;
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        glViewport(0, 0, NX, NY);
        for (int i = 0; i < 20; i++) {
            psi3 = psi1 - (imag_unit*(dt/hbar))*(h_psi_func(psi2, pot)
                                                 //+ 0.1*conj(psi2)*psi2*psi2
                                                 );
            swap(psi3, psi2);
            swap(psi1, psi3);
            // laplacian_psi2.paste_to_quad(main_frame);
        }
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        psi1.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        // (psi1 / 3.0).paste_to_quad(main_frame);
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}


int main() {
    #ifdef __APPLE__
    // int NX = 1440, NY = 1440;
    int NX = 1024, NY = 1024;
    #else
    int NX = 512, NY = 512;
    #endif
    GLFWwindow *window = init_window(NX, NY);
    init_bin_ops_programs();
    init_unary_ops_programs();
    init_fft_programs();
    // getchar();
    // complex_func(window, NX, NY);
    // poisson(window, NX, NY);
    // waves(window, NX, NY);
    // schrod_leapfrog(window, NX, NY);
    // schrod_splitstep(window, NX, NY);
    // stable_fluids(window, NX, NY);
    isf_splitstep(window, NX, NY);
    // dirac_leapfrog(window, NX, NY);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}



