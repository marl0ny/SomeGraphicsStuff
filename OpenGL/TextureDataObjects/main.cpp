#include <GLFW/glfw3.h>
#include "texture_data.hpp"
#include "draw_texture_data.hpp"


void complex_func_sim(GLFWwindow *window,
                      int window_width, int window_height) {
    int NX = window_width;
    int NY = window_height;
    frame_id main_frame = new_quad(NULL);
    int view_program = make_quad_program("./shaders/view.frag");
    auto x_float = make_x(-0.5, 0.5, FLOAT, NX, NY);
    auto y_float = make_y(-0.5, 0.5, FLOAT, NX, NY);
    auto x = x_float.cast_to(COMPLEX, Channel::X, Channel::NONE);
    auto iy = y_float.cast_to(COMPLEX, Channel::NONE, Channel::X);
    auto z = x + iy;
    auto f = 10.0*(z - 0.25)*(z + 0.25)*z;
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        bind_quad(main_frame, view_program);
        f.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        // (psi1 / 3.0).paste_to_quad(main_frame);
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
void schrod_sim(GLFWwindow *window, int window_width, int window_height) {
    int NX = window_width;
    int NY = window_height;
    frame_id main_frame = new_quad(NULL);
    double dt = 0.01;
    double hbar = 1.0;
    double m = 1.0;
    double width = 91.0, height = 91.0;
    double dx = width/(float)NX, dy = height/(float)NY;
    int view_program = make_quad_program("./shaders/view.frag");
    auto psi1 = Texture2DData(COMPLEX, NX, NY, true,
                              GL_REPEAT, GL_REPEAT,
                              GL_LINEAR, GL_LINEAR);
    auto command
        = DrawTexture2DData(Path("./shaders/init-gaussian.frag"));
    command.set_float_uniforms({{"amplitude", 5.0}, {"sigma", 0.05}});
    command.set_vec2_uniforms({{"r0", {0.2, 0.2}}, });
    command.set_ivec2_uniforms({{"wavenumber", {-15, 15}}, });
    command.draw(psi1);
    auto imag_unit = std::complex<double>(0.0, 1.0);
    auto x = make_x(-0.5, 0.5, FLOAT, NX, NY);
    auto y = make_y(-0.5, 0.5, FLOAT, NX, NY);
    auto pot = (20.0*(x*x + y*y)).cast_to(COMPLEX, Channel::X, Channel::NONE);
    auto h_psi_func = [=](Texture2DData &psi,
                          Texture2DData &pot) -> Texture2DData {
        auto laplacian_psi = psi.laplacian({.dx=dx, .dy=dy,
                .width=width, .height=height, .order_of_accuracy=4});
        return (-hbar*hbar)/(2.0*m)*laplacian_psi + pot*psi;
    };
    auto psi2 = psi1 - (imag_unit*(0.5*dt/hbar))*h_psi_func(psi1, pot);
    auto psi3 = psi2;
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        for (int i = 0; i < 20; i++) {
            psi3 = psi1 - imag_unit*(dt/hbar)*h_psi_func(psi2, pot);
            swap(psi3, psi2);
            swap(psi1, psi3);
            // laplacian_psi2.paste_to_quad(main_frame);
        }
        bind_quad(main_frame, view_program);
        psi1.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        // (psi1 / 3.0).paste_to_quad(main_frame);
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}


int main() {
    int NX = 512, NY = 512;
    GLFWwindow *window = init_window(NX, NY);
    init_bin_ops_programs();
    init_unary_ops_programs();
    // schrod_sim(window, NX, NY);
    complex_func_sim(window, NX, NY);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
