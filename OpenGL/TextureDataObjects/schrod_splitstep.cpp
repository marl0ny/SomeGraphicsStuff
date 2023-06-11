#include "schrod_splitstep.hpp"

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


/* Split operator implementation for the Schrodinger equation.

   References:

   - James Schloss. The Split-Operator Method. In The Arcane Algorithm Archive.
   https://www.algorithm-archive.org/contents/
   split-operator_method/split-operator_method.html.

   - Wikipedia contributors. (2021, May 6). Split-step method.
   In Wikipedia, The Free Encyclopedia.
   https://en.wikipedia.org/wiki/Split-step_method.

*/
int schrod_splitstep(GLFWwindow *window,frame_id main_frame) {
    int exit_status = 0;
    int NX = 512, NY = 512;
    int window_width = 0, window_height = 0;
    glfwGetWindowSize(window, &window_width, &window_height);
    // int NX = 256, NY = 256;
    int frame_count = 0;
    auto imag_unit = std::complex<double>(0.0, 1.0);
    // double dt = 3.0;
    std::complex<double> dt = 3.0 - 0.1*imag_unit;
    double hbar = 1.0;
    double m = 1.0;
    // double dx = width/(float)NX, dy = height/(float)NY;
    double pi = 3.141592653589793;
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    auto command
        = DrawTexture2DData(Path("./shaders/init-gaussian.frag"));
    command.set_float_uniforms({{"amplitude", 5.0},
                                {"sigmaX", 0.05}, {"sigmaY", 0.05}});
    command.set_vec2_uniforms({{"r0", {.x=0.25, .y=0.25}}, });
    command.set_ivec2_uniforms({{"wavenumber", {.x=-5, .y=5}}, });
    auto psi = command.create(COMPLEX, NX, NY, true,
                               GL_REPEAT, GL_REPEAT,
                               GL_LINEAR, GL_LINEAR);
    auto psi1 = psi*(100.0/sqrt(psi.squared_norm().as_double));
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
    auto view_com = DrawTexture2DData(Path("./shaders/view.frag"));
    auto data = new uint8_t[3*window_width*window_height] {0,};
    std::time_t t0 = std::time(nullptr);
    for (int k = 0, exit_loop = false; !exit_loop; k++) {
        glViewport(0, 0, NX, NY);
        for (int i = 0; i < 1; i++) {
            psi2 = h_psi_func(psi1, pot);
            psi2 = psi2*(100.0/sqrt(psi2.squared_norm().as_double));
            swap(psi1, psi2);
        }
        glViewport(0, 0, window_width, window_height);
        /*{
            auto bmp_frame = Texture2DData(HALF_FLOAT3,
                                           window_width, window_height);
            view_com.draw(bmp_frame, "tex", psi1);

            bmp_frame.paste_to_rgb_image_data(data);
            std::stringstream filename {};
            std::string d_path = "./";
            if (k < 10)
                filename << d_path << "data_000" << k << ".bmp";
            else if (k < 100)
                filename << d_path << "data_00" << k << ".bmp";
            else if (k < 1000)
                filename << d_path << "data_0" << k << ".bmp";
            else
                filename << d_path << "data_" << k << ".bmp";
            std::string filename_str = filename.str();
            write_to_bitmap(&filename_str[0], data,
            window_width, window_height);

        }*/
        bind_quad(main_frame, view_program);
        psi1.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        glfwPollEvents();
        frame_count++;
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && k > 30)
            exit_loop = true;
        if (glfwWindowShouldClose(window)) {
            exit_loop = true;
            exit_status = 1;
        }
        glfwSwapBuffers(window);
    }
    std::time_t t1 = std::time(nullptr);
    std::cout << (double)frame_count/((double)t1 - (double)t0) << std::endl;
    return exit_status;
}

int isf_leapfrog(GLFWwindow *window, frame_id main_frame) {
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    int NX = 256;
    int NY = 256;
    double dt = 0.01;
    double hbar = 1.0;
    double m = 1.0;
    double width = ((double)NX/512.0)*91.0;
    double height = ((double)NX/512.0)*91.0;
    double dx = width/(float)NX, dy = height/(float)NY;
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
    auto dist0 = init_dist_command.create(FLOAT, NX, NY);
    auto tmp0 = init_current_command.create(COMPLEX, NX, NY);
    init_dist_command.set_float_uniforms({{"amplitude", -5.0}});
    init_current_command.set_vec2_uniforms({{"r0", {.x=0.75, .y=0.75}}});
    init_dist_command.set_vec2_uniforms({{"r0", {.x=0.75, .y=0.75}}});
    init_current_command.set_ivec2_uniforms({{"wavenumber",
                                             {.x=-30, .y=-30}}});
    auto dist1 = init_dist_command.create(FLOAT, NX, NY);
    auto tmp1 = init_current_command.create(COMPLEX, NX, NY);
    auto dist = dist0 + dist1;
    auto psi1_u = 0.99*tmp0*tmp1;
    auto psi1_d = sqrt(1.0 - conj(psi1_u)*psi1_u);
    auto imag_unit = std::complex<double>(0.0, 1.0);


    auto poisson_solve_com
        = DrawTexture2DData(Path("./shaders/poisson-jacobi.frag"));
    auto poisson_func = [&](Texture2DData &rho,
                            Texture2DData &x0,
                            int n_iterations) -> Texture2DData {
        poisson_solve_com.set_float_uniforms({{"dx", dx}, {"dy", dy},
                                              {"width", width},
                                              {"height", height}});
        auto x = x0;
        for (int i = 0; i < n_iterations; i++)
            poisson_solve_com.draw(x, "prevIterTex", x, "bTex", rho);
        return x;
    };

    auto current_func = [=](Texture2DData &psi_u,
                            Texture2DData &psi_d
                            ) -> std::vector<Texture2DData> {
        Grad2DParams grad_params {.dx=dx, .dy=dy,
            .width=width, .height=height,
            .order_of_accuracy=4, .staggered=1};
        auto hbar_i = -imag_unit*hbar;
        auto psi_u_dag
            = roll(conj(psi_u), -dx/(2.0*width), -dy/(2.0*height));
        auto psi_d_dag
            = roll(conj(psi_d), -dx/(2.0*width), -dy/(2.0*height));
        auto jx = (hbar_i*(psi_u_dag*ddx(psi_u, grad_params)
                           + psi_d_dag*ddx(psi_d, grad_params))
                   ).cast_to(COMPLEX, X, NONE);
        auto jy = (hbar_i*(psi_u_dag*ddy(psi_u, grad_params)
                           + psi_d_dag*ddy(psi_d, grad_params))
                   ).cast_to(COMPLEX, X, NONE);
        return std::vector<Texture2DData> {jx, jy};
    };

    auto pressure_func = [=](std::vector<Texture2DData> &j,
                             Texture2DData &pressure) -> Texture2DData {
        Grad2DParams grad_params {.dx=dx, .dy=dy,
            .width=width, .height=height,
            .order_of_accuracy=4, .staggered=-1};
        auto grad_dot_j = ddx(j[0], grad_params) + ddy(j[1], grad_params);
        auto pressure_f = pressure.cast_to(FLOAT, X);
        return poisson_func(grad_dot_j,
                            pressure_f, 10).cast_to(COMPLEX, X, NONE);
    };

    auto h_psi_func = [&](Texture2DData &psi,
                          Texture2DData &potential) -> Texture2DData {
        auto laplacian_psi = psi.laplacian({.dx=dx, .dy=dy,
                .width=width, .height=height, .order_of_accuracy=4});
        return ((-hbar*hbar)/(2.0*m))*laplacian_psi + potential*psi;
    };

    auto step_func = [=](Texture2DData &psi1_u, Texture2DData &psi1_d,
                         Texture2DData &psi2_u, Texture2DData &psi2_d,
                         Texture2DData &psi3_u, Texture2DData &psi3_d,
                         Texture2DData &pressure, double dt) {
        auto current = current_func(psi2_u, psi2_d);
        pressure = pressure_func(current, pressure);
        psi3_u = psi1_u - (imag_unit*(dt/hbar))*(h_psi_func(psi2_u,
                                                            pressure));
        psi3_d = psi1_d - (imag_unit*(dt/hbar))*(h_psi_func(psi2_d,
                                                            pressure));
        Texture2DData norm = sqrt(psi3_u*conj(psi3_u) + psi3_d*conj(psi3_d));
        psi3_u = psi3_u / norm;
        psi3_d = psi3_d / norm;
    };

    auto pressure = 0.0*psi1_u;
    auto psi2_u = psi1_u, psi2_d = psi1_d;
    step_func(psi1_u, psi1_d, psi1_u, psi1_d, psi2_u, psi2_d,
              pressure, dt/2.0);
    auto psi3_u = psi2_u, psi3_d = psi2_d;

    for (int k = 0, exit_loop=false; !exit_loop; k++) {
        glViewport(0, 0, NX, NY);
        for (int i = 0; i < 6; i++) {
            // time step
            step_func(psi1_u, psi1_d, psi2_u, psi2_d, psi3_u, psi3_d,
                      pressure, dt);
            // swap stuff
            swap(psi3_u, psi2_u);
            swap(psi1_u, psi3_u);
            swap(psi3_d, psi2_d);
            swap(psi1_d, psi3_d);
        }
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        psi1_u.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && k > 30)
            exit_loop = true;
        if (glfwWindowShouldClose(window)) {
            exit_status = 1;
            exit_loop = true;
        }
        glfwSwapBuffers(window);
    }
    return exit_status;
}
