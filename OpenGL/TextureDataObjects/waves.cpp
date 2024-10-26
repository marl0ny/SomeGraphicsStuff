/* Finite difference waves simulation using
second order centred difference in time.*/
#include "waves.hpp"

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
#include "render.hpp"
#include "interactor.hpp"
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

int waves(Renderer *renderer) {
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    GLFWwindow *window = renderer->window;
    frame_id main_frame = renderer->main_frame;
    window_dimensions(window, &window_width, &window_height);
    int NX = window_width, NY = window_height;
    int view_program = make_quad_program("./shaders/view.frag");
    Texture2DData x = funcs2D::make_x(-0.5, 0.5, FLOAT, NX, NY);
    Texture2DData y = funcs2D::make_y(-0.5, 0.5, FLOAT, NX, NY);
    auto w1 = 10.0*exp(-0.5*(x*x + y*y)/(0.01*0.01));
    /* w1 = w1 + 10.0*exp((-0.5/(0.01*0.01))*((x-0.3)*(x-0.3)
         + (y+0.1)*(y+0.1)));
    w1 = w1 + 10.0*exp((-0.5/(0.01*0.01))*((x+0.3)*(x+0.3)
         + (y-0.3)*(y-0.3)));
    w1 = w1 + 10.0*exp((-0.5/(0.01*0.01))*((x+0.1)*(x+0.1)
         + (y+0.3)*(y+0.3)));*/
    // w1 = w1.cast_to(COMPLEX, Channel::NONE, Channel::X);
    auto w2 = w1;
    auto w3 = w2;
    auto step = [=](Texture2DData &w0, Texture2DData &w1,
                    double dt) -> Texture2DData {
        auto laplacian_w1 = funcs2D::laplacian(w1, {.dx=1.0, .dy=1.0,
                .width=(double)NX, .height=(double)NY,
                .order_of_accuracy=8});
        return dt*laplacian_w1 + 2.0*w1 - w0;
    };

    int k = 0;
    bool exit_loop = false;
    loop = [&] {
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
        Interactor interactor(window);
        interactor.click_update(renderer);
        if (interactor.left_pressed()) {
            DVec2 left_click = interactor.get_mouse_position();
            // std::cout << left_click.x << std::endl;
            double bx = left_click.x - 0.5;
            double by = left_click.y - 0.5;
            auto u = 3.0*exp((-0.5/(0.01*0.01))
                                            *((x-bx)*(x-bx) + (y-by)*(y-by)));
            w1 = w1 + u;
            w2 = w2 + u;
        }
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && k > 30)
            exit_loop = true;
        if (glfwWindowShouldClose(window)) {
            exit_status = 1;
            exit_loop = true;
        }
        glfwSwapBuffers(window);
        k++;
        #ifdef __EMSCRIPTEN__
        if (exit_loop)
            emscripten_cancel_main_loop();
        #endif
    };
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
    #else
    while (!exit_loop)
        loop();
    #endif
    // std::cout << "This is reached\n";

    return exit_status;
}
