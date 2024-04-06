#include "schrod_leapfrog.hpp"


// #include <OpenGL/OpenGL.h>
#include <cstdlib>
#include <new>
#define GL_SILENCE_DEPRECATION

#include "texture_data.hpp"
#include "draw_texture_data.hpp"
// #include <OpenGL/OpenGL.h>
#include "vector_field_view_2d.hpp"
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


/* Numerically solving the Schrodinger equation using explicit finite
   differences. This follows an article by Visscher.

   Reference:

   Visscher, P. (1991).
   A fast explicit algorithm for the time‐dependent Schrödinger equation.
   Computers in Physics, 5<, 596-598.
   https://doi.org/10.1063/1.168415

   Probability Current:

   Wikipedia - Probability current
   https://en.wikipedia.org/wiki/Probability_current



*/
int schrod_leapfrog(Renderer *renderer) {
    int main_frame = renderer->main_frame;
    GLFWwindow *window = renderer->window;
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    // int NX = 64;
    // int NY = 64;
    int NX = 256;
    int NY = 256;
    double dt = 0.011;
    double hbar = 1.0;
    double m = 1.0;
    double width = ((double)NX/512.0)*91.0;
    double height = ((double)NX/512.0)*91.0;
    double dx = width/(float)NX, dy = height/(float)NY;
    int view_program = make_quad_program("./shaders/view.frag");
    int copy_program = make_quad_program("./shaders/util/copy.frag");
    glViewport(0, 0, NX, NY);
    auto command
        = DrawTexture2DData(Path("./shaders/wavepacket/init.frag"));
    command.set_float_uniforms({{"amplitude", 5.0},
				{"sigmaX", 0.05}, {"sigmaY", 0.05}});
    command.set_vec2_uniforms({{"r0", {.x=0.2, .y=0.2}}, });
    command.set_ivec2_uniforms({{"wavenumber", {.x=-5, .y=5}}, });
    auto psi1 = command.create(COMPLEX, NX, NY, true,
                               GL_REPEAT, GL_REPEAT,
                               GL_LINEAR, GL_LINEAR);
    auto imag_unit = std::complex<double>(0.0, 1.0);
    auto x = funcs2D::make_x(-0.5, 0.5, FLOAT, NX, NY);
    auto y = funcs2D::make_y(-0.5, 0.5, FLOAT, NX, NY);
    auto pot = (20.0*(x*x + y*y)).cast_to(COMPLEX, X, NONE);
    auto h_psi_func = [=](Texture2DData &psi,
                          Texture2DData &pot) -> Texture2DData {
        auto laplacian_psi = funcs2D::laplacian(psi, {.dx=dx, .dy=dy,
                .width=width, .height=height, .order_of_accuracy=4});
        return ((-hbar*hbar)/(2.0*m))*laplacian_psi + pot*psi;
    };
    auto psi2 = psi1 - (imag_unit*(0.5*dt/hbar))*h_psi_func(psi1, pot);
    auto psi3 = psi2;

    auto compute_current = [&](const Texture2DData &psi) -> Texture2DData {
        Grad2DParams params = {.dx=dx, .dy=dy, 
                               .width=width, .height=height,
                               .staggered=1, .order_of_accuracy=4};
        auto ddx_psi = funcs2D::ddx(psi, params);
        auto ddy_psi = funcs2D::ddy(psi, params);
        auto jx = (hbar/m)*(conj(psi)*ddx_psi).cast_to(FLOAT, Y);
        auto jy = (hbar/m)*(conj(psi)*ddy_psi).cast_to(FLOAT, Y);
        auto j = jx.cast_to(FLOAT2, X, NONE) 
                                + jy.cast_to(FLOAT2, NONE, X);
        return j;
    };

    // auto psi_p = ifft(fft(psi3));
    // std::cout << psi_p.get_frame_id() << std::endl;
    auto view_com = DrawTexture2DData(Path("./shaders/view.frag"));
    int frame_count = 0;
    auto data = new uint8_t[3*window_width*window_height] {0,};
    std::time_t t0 = std::time(nullptr);

    auto vec_view = VectorFieldView2D(window_width, window_height, 64, 64);
    auto ones = funcs2D::make_x(1.0, 1.0, FLOAT2, NX, NY);

    int k = 0;
    int exit_loop = false;
    loop = [&] {
        glViewport(0, 0, NX, NY);
        for (int i = 0; i < 20; i++) {
            psi3 = psi1 - (imag_unit*(dt/hbar))*(h_psi_func(psi2, pot)
                                                 //+ 0.1*conj(psi2)*psi2*psi2
                                                 );
            swap(psi3, psi2);
            swap(psi1, psi3);
            // laplacian_psi2.paste_to_quad(main_frame);
        }
        auto j = compute_current(psi1);
        auto f = 0.1*j/(conj(psi1)*psi1).cast_to(FLOAT2, X, X);
        auto view = vec_view.render((psi1*conj(psi1)).cast_to(FLOAT3, X, X, X), 0.01*j);
        glViewport(0, 0, window_width, window_height);
        frame_count++;
        
        bind_quad(main_frame, copy_program);
        view.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();

        /* bind_quad(main_frame, view_program);
        psi1.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();*/

        // (psi1 / 3.0).paste_to_quad(main_frame);
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

    std::time_t t1 = std::time(nullptr);
    std::cout << (double)frame_count/((double)t1 - (double)t0) << std::endl;
    return exit_status;
}
