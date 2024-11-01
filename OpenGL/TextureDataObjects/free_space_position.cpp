/* Simulation of a Gaussian wave packet in 2D in the absence of any potential.

The equation of motion for a Gaussian wave packet in free space is derived 
in pg 151-154 of Shankar, with the final result in eq. 5.1.15. This was
checked with Sympy by integrating an initial Gaussian wave packet with the 
free-space position space propagator (5.1.10) 
(integral expression in eq. 5.1.11).

Reference:
Shankar, R. (1994). Simple Problems in One Dimension.
In Principles of Quantum Mechanics, chapter 5. Springer.
*/

#include "free_space_position.hpp"

#define GL_SILENCE_DEPRECATION

#include "texture_data.hpp"
#include "draw_texture_data.hpp"
#include "interactor.hpp"
// #include <OpenGL/OpenGL.h>

#include "render.hpp"
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


int free_space_position(Renderer *renderer) {
    int main_frame = renderer->main_frame;
    GLFWwindow *window = renderer->window;
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    int NX = 512;
    int NY = 512;
    double hbar = 1.0;
    double m = 1.0;
    double c = 137.036;
    double dt = 0.001;
    float width = (float)10.0;
    float height = (float)10.0;
    double t = 0.0;
    float r0x = 0.5*width, r0y = 0.5*height;
    float px = 10.0, py = 10.0;
    double amplitude = 10.0;

    // double dx = width/(float)NX, dy = height/(float)NY;
    // auto imag_unit = std::complex<double>(0.0, 1.0);
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    // auto x = funcs2D::make_x(0.0, 2.0, FLOAT, NX, NY);
    // auto y = funcs2D::make_y(0.0, 2.0, FLOAT, NX, NY);
    auto psi = funcs2D::zeroes(FLOAT2, NX, NY);

    auto drawer 
        = Drawer(Path("./shaders/free-space-position/schrodinger.frag"));

    int k = 0;
    bool exit_loop = false;
    Interactor interactor = Interactor(window);
    loop = [&] {
        glViewport(0, 0, NX, NY);

        drawer.draw(
            psi,
            {
                {"hbar", {float(hbar)}},
                {"m", {float(m)}},
                // {"c", {float(c)}},
                {"t", {float(t)}},
                {"amplitude", {float(amplitude)}},
                // {"texDimensions2D", {IVec2 {NX, NY}}},
                {"r0", {Vec3 {r0x, r0y, 0.0}}},
                {"p0", {Vec3 {px, py, 0.0}}},
                {"sigma", {Vec3 {0.01F*width, 0.01F*height, 0.0}}},
                {"dimensions", {Vec3 {width, height, 0.0}}}
            }
        );
        t += dt;

        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        psi.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        glfwPollEvents();
        interactor.click_update(renderer);
        if (interactor.left_pressed()) {
            DVec2 mouse_position = interactor.get_mouse_position();
            DVec2 mouse_delta = interactor.get_mouse_delta();
            px = (float)mouse_delta.x*width*100.0;
            py = (float)mouse_delta.y*height*100.0;
            r0x = (float)mouse_position.x*width;
            r0y = (float)mouse_position.y*height;
            t = 0.0;

        }
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
    while(!exit_loop)
        loop();
    #endif
    return exit_status;
}
 