/* Numerically solving the Schrodinger equation in 3D using explicit finite
   differences. This follows an article by Visscher.

   Reference:

   Visscher, P. (1991).
   A fast explicit algorithm for the time‐dependent Schrödinger equation.
   Computers in Physics, 5<, 596-598.
   https://doi.org/10.1063/1.168415

*/
#include "dirac_leapfrog.hpp"

// #include <OpenGL/OpenGL.h>
#include <GL/gl.h>
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

int schrod_leapfrog_3d(Renderer *renderer) {
    int main_frame = renderer->main_frame;
    GLFWwindow *window = renderer->window;

    int exit_status = 0;

    // Get the screen dimensions
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);

    // Define some constants
    int NX = 128;
    int NY = 128;
    int NZ = 128;
    double hbar = 1.0;
    double m = 1.0;
    double dt = 0.1;
    double width = (float)NX;
    double height = (float)NY;
    double dx = width/(float)NX, dy = height/(float)NY, dz = height/(float)NZ;
    auto imag_unit = std::complex<double>(0.0, 1.0);

    // Texture for the potential
    use_3d_texture(NX, NY, NZ);

    // Texture to store initial wave function
    // auto psi1 = funcs3D::zeroes(COMPLEX2, NX, NY, NZ);
    auto x = funcs3D::make_x(
        -0.5, 0.5, FLOAT, NX, NY, NZ).cast_to(COMPLEX2, X, NONE, NONE, NONE);
    auto y = funcs3D::make_y(
        -0.5, 0.5, FLOAT, NX, NY, NZ).cast_to(COMPLEX2, X, NONE, NONE, NONE);
    auto z = funcs3D::make_z(
        -0.5, 0.5, FLOAT, NX, NY, NZ).cast_to(COMPLEX2, X, NONE, NONE, NONE);
   // auto psi1 = 10.0*z;
    auto i_tau = std::complex<double>(0.0, 2.0*3.14159);
    double sx = 0.05, sy = 0.05, sz = 0.05;
    // double sx = 0.1, sy = 0.1, sz = 0.;
    auto xs = x - 0.2;
    auto ys = y - 0.2;
    auto zs = z - 0.2;
    auto pot = 4.0*(x*x + y*y + z*z);
    auto psi1 = 
        10.0*exp(i_tau*(xs + ys + zs))
        *exp(-0.5*(xs*xs/(sx*sx) + ys*ys/(sy*sy) + zs*zs/(sz*sz)));

    // Create a texture that will store the Laplacian of psi
    auto laplacian_psi = funcs3D::zeroes(COMPLEX2, NX, NY, NZ);

    // Laplacian program
    auto laplacian_command
         = DrawTexture2DData(Path("./shaders/laplacian3d.frag"));
    laplacian_command.set_ivec2_uniforms({
        {"texelDimensions2D", {NX*NZ, NY}},
    });
    laplacian_command.set_ivec3_uniforms({
        {"texelDimensions3D", {NX, NY, NZ}},
    });
    laplacian_command.set_vec3_uniforms({
        {"dr", {(float)dx, (float)dy, (float)dz}},
        {"dimensions3D", {(float)NX, (float)NY, (float)NZ}},
    });

    auto h_psi_func = [=](Texture2DData &psi,
                          Texture2DData &pot) -> Texture2DData {
        laplacian_command.draw(laplacian_psi, "tex", psi);
        return ((-hbar*hbar)/(2.0*m))*laplacian_psi + pot*psi;
    };

    // Do the first step
    auto psi2 = psi1 - (imag_unit*(0.5*dt/hbar))*h_psi_func(psi1, pot);
    auto psi3 = psi2;

    int view_program = make_quad_program("./shaders/view.frag");

    int k = 0;
    bool exit_loop = false;
    loop = [&] {
        
        use_3d_texture(NX, NY, NZ);
        for (int i = 0; i < 10; i++) {
            psi3 = psi1 - (imag_unit*dt/hbar)*h_psi_func(psi2, pot);
            swap(psi3, psi2);
            swap(psi1, psi3);
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
    };

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
    #else
    while (!exit_loop)
        loop();
    #endif

    return exit_status;
}