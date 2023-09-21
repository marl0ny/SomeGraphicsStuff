#include "summation_stuff.hpp"

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


/* Sum through all elements in a texture. */
int summation_stuff(GLFWwindow *window,
                    frame_id main_frame) {
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    int NX = window_width, NY = window_height;
    int view_program = make_quad_program("./shaders/view.frag");
    auto *data = new Vec4[NX*NY] {0.,};
    struct FillDataRet {
        DVec4 elementwise_sum;
        double norm_squared;
    };
    auto fill_data = [&]() -> FillDataRet {
        DVec4 sum {.x=0.0, .y=0.0, .w=0.0, .z=0.0};
        double norm_squared_sum = 0.0;
        for (int i = 0; i < NY; i++) {
            for (int j = 0; j < NX; j++) {
                double x = 2.0*(double)rand()/RAND_MAX - 1.0;
                double y = 2.0*(double)rand()/RAND_MAX - 1.0;
                double z = 2.0*(double)rand()/RAND_MAX - 1.0;
                double w = 2.0*(double)rand()/RAND_MAX - 1.0;
                data[i*NX + j].x = x;
                data[i*NX + j].y = y;
                data[i*NX + j].z = z;
                data[i*NX + j].w = w;
                sum.x += x;
                sum.y += y;
                sum.z += z;
                sum.w += w;
                norm_squared_sum += x*x + y*y + z*z + w*w;
            }
        }
        return {
            .elementwise_sum = sum,
            .norm_squared = norm_squared_sum,
        };
    };

    int k = 0;
    bool exit_loop = false;
    loop = [&] {
        FillDataRet tmp = fill_data();
        DVec4 sum = tmp.elementwise_sum;
        double norm_squared_sum = tmp.norm_squared;
        auto f = Texture2DData(data, NX, NY);
        PixelData sum2 = f.sum_reduction();
        double tmp2 = f.squared_norm().as_double;

        // Print the information
        std::cout << "Array sum for loop:" << std::endl;
        std::cout << "x: " << sum.x << ", y: " << sum.y
         << ", z: " << sum.z << ", w: " << sum.w << std::endl;
        std::cout << "Using method: " << std::endl;
        std::cout << "x: " << sum2.as_dvec4.x << ", y: " << sum2.as_dvec4.y
         << ", z: " << sum2.as_dvec4.z << ", w: " << sum2.as_dvec4.w
         << std::endl;
        std::cout << "Norm squared for loop: ";
        std::cout << norm_squared_sum << std::endl;
        std::cout << "Using method: ";
        std::cout << tmp2 << std::endl;

        bind_quad(main_frame, view_program);
        f.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        // sum();
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
    return exit_status;
}
