#include "complex_func.hpp"

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

/* Plot and display complex functions.
*/
int complex_func(GLFWwindow *window, frame_id main_frame) {
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    int NX = window_width;
    int NY = window_height;
    std::complex<double> imag_unit {0.0, 1.0};
    int view_program = make_quad_program("./shaders/view.frag");
    auto x_float = funcs2D::make_x(-0.5, 0.5, FLOAT, NX, NY);
    auto y_float = funcs2D::make_y(-0.5, 0.5, FLOAT, NX, NY);
    auto x = x_float.cast_to(COMPLEX, X, NONE);
    auto y = y_float.cast_to(COMPLEX, X, NONE);
    auto z = x + imag_unit*y;
    auto f = sin(3.14159*z);
    // auto f = 10.0*(z - 0.25)*(z + 0.25)*z;
    for (int k = 0, exit_loop = false; !exit_loop; k++) {
        bind_quad(main_frame, view_program);
        f.set_as_sampler2D_uniform("tex");
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
