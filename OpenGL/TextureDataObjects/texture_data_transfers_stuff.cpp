#include "texture_data_transfers_stuff.hpp"

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


int substitute_data_from_cpu(Renderer *renderer) {
    int main_frame = renderer->main_frame;
    GLFWwindow *window = renderer->window;
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    int NX = window_width;
    int NY = window_height;
    int view_program = make_quad_program("./shaders/view.frag");
    auto *data = new std::complex<float>[NX*NY] {0.,};
    for (int i = 0; i < NX; i++) {
        for (int j = 0; j < NY; j++) {
            data[i*NY + j].real((float)j/(float)NX);
            data[i*NX + j].imag((float)i/(float)NY);
        }
    }
    auto f = Texture2DData(data, NX, NY);
    auto f2 = pow(f, 2.0);
    for (int k = 0, exit_loop=false; !exit_loop; k++) {
        bind_quad(main_frame, view_program);
        f2.set_as_sampler2D_uniform("tex");
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

int copy_data_for_cpu(Renderer *renderer) {
    int main_frame = renderer->main_frame;
    GLFWwindow *window = renderer->window;
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    int NX = window_width;
    int NY = window_height;
    int view_program = make_quad_program("./shaders/view.frag");
    auto *data = new uint8_t [NX*NY*3] {0,};
    auto x0 = funcs2D::make_x(0.0, 1.0, FLOAT, NX, NY); // - 0.5 - 0.5/(double)NX;
    auto y0 = funcs2D::make_y(0.0, 1.0, FLOAT, NX, NY);
    auto f = x0 + y0;
    for (int i = 0; i < NY; i++) {
        for (int j = 0; j < NX; j++) {
            data[3*(i*NX + j)] = 255;
            data[3*(i*NX + j) + 1] = 255;
            data[3*(i*NX + j) + 2] = 255;
        }
    }
    write_to_bitmap("data.bmp", data, NX, NY);
    for (int k = 0, exit_loop=false; !exit_loop; k++) {
        bind_quad(main_frame, view_program);
        f.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        {
            auto f2 = f.cast_to(UBYTE3, B, G, R);
            f2.paste_to_array(data);
            std::stringstream filename {};
            filename << "data_" << k << ".bmp";
            std::string filename_str = filename.str();
            write_to_bitmap(&filename_str[0], data, NX, NY);
        }
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && k > 30)
            exit_loop = true;
        if (glfwWindowShouldClose(window)) {
            exit_status = 1;
            exit_loop = true;
        }
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    return exit_status;
}
