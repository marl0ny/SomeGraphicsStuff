#include "schrod_splitstep_3d.hpp"

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
#include <time.h>

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


static double time_difference_in_ms(const struct timespec *t1,
                                    const struct timespec *t2) {
    if (t2->tv_nsec >= t1->tv_nsec)
        return (double)(t2->tv_nsec - t1->tv_nsec)/1000000.0;
    return (double)(999999999 - t1->tv_nsec + t2->tv_nsec)/1000000.0;
}

/* Split operator implementation for the Schrodinger equation in 3D.

   References:

   - James Schloss. The Split-Operator Method. In The Arcane Algorithm Archive.
   https://www.algorithm-archive.org/contents/
   split-operator_method/split-operator_method.html.

   - Wikipedia contributors. (2021, May 6). Split-step method.
   In Wikipedia, The Free Encyclopedia.
   https://en.wikipedia.org/wiki/Split-step_method.
   
*/
int schrod_splitstep_3d(Renderer *renderer) {
    int main_frame = renderer->main_frame;
    GLFWwindow *window = renderer->window;

    int exit_status = 0;

    // Get the screen dimensions
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);

    // Define some constants
    int NX = 64;
    int NY = 64;
    int NZ = 64;
    double hbar = 1.0;
    double m = 1.0;
    double dt = 1.0;
    double width = (float)NX;
    double height = (float)NY;
    double dx = width/(float)NX, dy = height/(float)NY, dz = height/(float)NZ;
    auto imag_unit = std::complex<double>(0.0, 1.0);

    use_3d_texture(NX, NY, NZ);

    // Texture to store initial wave function
    // auto psi1 = funcs3D::zeroes(COMPLEX2, NX, NY, NZ);
    auto x = funcs3D::make_x(
        -0.5, 0.5, FLOAT, NX, NY, NZ).cast_to(COMPLEX, X, NONE);
    auto y = funcs3D::make_y(
        -0.5, 0.5, FLOAT, NX, NY, NZ).cast_to(COMPLEX, X, NONE);
    auto z = funcs3D::make_z(
        -0.5, 0.5, FLOAT, NX, NY, NZ).cast_to(COMPLEX, X, NONE);
    // auto psi1 = 10.0*z;
    double pi = 3.14159;
    auto i_tau = std::complex<double>(0.0, 2.0*pi);
    auto px = funcs3D::fftshift(2.0*pi*x).cast_to(COMPLEX, X, NONE);
    auto py = funcs3D::fftshift(2.0*pi*y).cast_to(COMPLEX, X, NONE);
    auto pz = funcs3D::fftshift(2.0*pi*z).cast_to(COMPLEX, X, NONE);
    auto p2 = px*px + py*py + pz*pz;
    auto p_propagator = exp((-imag_unit*dt/(2.0*m*hbar))*p2);
    double sx = 0.1, sy = 0.1, sz = 0.1;
    auto xs = x - 0.2;
    auto ys = y - 0.2;
    auto zs = z - 0.2;
    auto pot = 4.0*powf(64.0F/(float)NZ, 2.0)*(x*x + y*y + z*z);
    auto x_propagator = exp((-imag_unit*(dt/2.0)/hbar)*pot);
    auto psi1 = 
        10.0*exp(i_tau*(xs + ys + zs))
        *exp(-0.5*(xs*xs/(sx*sx) + ys*ys/(sy*sy) + zs*zs/(sz*sz)));
    // auto psi2 = funcs3D::ifft(funcs3D::fft(psi1));

    int view_program = make_quad_program("./shaders/view.frag");

    int k = 0;
    bool exit_loop = false;
    loop = [&] {

        struct timespec frame_start, frame_end;
        clock_gettime(CLOCK_MONOTONIC, &frame_start);

        use_3d_texture(NX, NY, NZ);
        int steps_per_frame = 1;
        for (int i = 0; i < steps_per_frame; i++) {
            auto psi_p = funcs3D::fft(x_propagator*psi1);
            psi1 = x_propagator*funcs3D::ifft(psi_p*p_propagator);
        }


        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        psi1.set_as_sampler2D_uniform("tex");
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
        if (k % 10 == 0 && k != 0) {
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