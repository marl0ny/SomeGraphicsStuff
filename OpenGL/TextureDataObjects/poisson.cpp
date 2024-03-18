#include "poisson.hpp"

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


/* A rudimentary Poisson solver.
 *
 * References:
 *
 * - Wikipedia contributors. (2023, March 9). Poisson's equation.
 *   In Wikipedia, The Free Encyclopedia.
 */
int poisson(GLFWwindow *window, frame_id main_frame) {
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    int NX = 128, NY = 128;
    // double width = (double)NX, height = (double)NY;
    // double dx = width/(double)NX, dy = height/(double)NY;
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    auto x0 = funcs2D::make_x(0.0, 1.0, 
                                             FLOAT, NX, NY
                                             ) - 0.5 - 0.5/(double)NX;
    auto y0 = funcs2D::make_y(0.0, 1.0, 
                                             FLOAT, NX, NY
                                             ) - 0.5 - 0.5/(double)NY;
    // auto x = max(x0, 0.5/(double)NX).cast_to(COMPLEX, X, NONE);
    // auto y = max(y0, 0.5/(double)NY).cast_to(COMPLEX, X, NONE);
    auto x = x0; // .cast_to(COMPLEX, X, NONE);
    auto y = y0; // .cast_to(COMPLEX, X, NONE);
    double pi = 3.141592653589793;
    auto px = funcs2D::fftshift(2.0*pi*x);
    auto py = funcs2D::fftshift(2.0*pi*y);
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
    auto phi = -1.0*funcs2D::ifft(funcs2D::fft(w)/eigval);
    phi = phi.cast_to(COMPLEX, X, Y, NONE, NONE);
    // auto laplacian_phi = 100.0*phi.laplacian({.dx=dx, .dy=dy,
    //         .width=width, .height=height, .order_of_accuracy=4});
    for (int k = 0, exit_loop=false; !exit_loop; k++) {
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        phi.set_as_sampler2D_uniform("tex");
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
