#include "schrod_leapfrog.hpp"


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


/* Numerically solving the Schrodinger equation using explicit finite
   differences. This follows an article by Visscher.

   Reference:

   Visscher, P. (1991).
   A fast explicit algorithm for the time‐dependent Schrödinger equation.
   Computers in Physics, 5<, 596-598.
   https://doi.org/10.1063/1.168415

*/
int schrod_leapfrog(GLFWwindow *window,
                    frame_id main_frame) {
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    int NX = 512;
    int NY = 512;
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
    auto view_com = DrawTexture2DData(Path("./shaders/view.frag"));
    int frame_count = 0;
    auto data = new uint8_t[3*window_width*window_height] {0,};
    std::time_t t0 = std::time(nullptr);
    for (int k = 0, exit_loop=false; !exit_loop; k++) {
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
        /*{
            auto bmp_frame = Texture2DData(HALF_FLOAT3,
                                           window_width, window_height);
            view_com.draw(bmp_frame, "tex", psi1);

            bmp_frame.paste_to_rgb_image_data(data);
            std::stringstream filename {};
            if (k < 10)
                filename << "data_000" << k << ".bmp";
            else if (k < 100)
                filename << "data_00" << k << ".bmp";
            else if (k < 1000)
                filename << "data_0" << k << ".bmp";
            else
                filename << "data_" << k << ".bmp";
            std::string filename_str = filename.str();
            write_to_bitmap(&filename_str[0], data,
            window_width, window_height);

        }*/
        frame_count++;
        bind_quad(main_frame, view_program);
        psi1.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        // (psi1 / 3.0).paste_to_quad(main_frame);
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && k > 30)
            exit_loop = true;
        if (glfwWindowShouldClose(window)) {
            exit_status = 1;
            exit_loop = true;
        }
        glfwSwapBuffers(window);
    }
    std::time_t t1 = std::time(nullptr);
    std::cout << (double)frame_count/((double)t1 - (double)t0) << std::endl;
    return exit_status;
}
