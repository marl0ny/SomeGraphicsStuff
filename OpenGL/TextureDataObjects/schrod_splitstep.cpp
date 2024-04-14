#include "schrod_splitstep.hpp"

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
                                    const struct timespec *t2);



float p(int i, float len) {
    return 2.0*3.14159*float(i)/len;
}

void init_wavepacket_in_momentum_space(const Texture2DData &t) {
    auto init_wavepacket_proc = Drawer(
        Path("./shaders/wavepacket/init-in-momentum-space.frag"));
    init_wavepacket_proc.draw(
        t,
        {
            {"position", {Vec3 {0.25, 0.25, 0.0}}},
            {"sigma", {Vec3 {p(50, 1.0), p(50, 1.0), 1.0}}},
            {"momentum", {Vec3 {p(20, 1.0), p(20, 1.0), 0}}},
            {"dimensions", {Vec3 {1.0, 1.0, 0.0}}},
            {"texelDimensions", {IVec3 {256, 256, 0}}},
            {"spinor", {Vec4 {1.0, 0.0, 0.0, 0.0}}},
        }
    );
}


/* Split operator implementation for the Schrodinger equation.

   References:

   - James Schloss. The Split-Operator Method. In The Arcane Algorithm Archive.
   https://www.algorithm-archive.org/contents/
   split-operator_method/split-operator_method.html.

   - Wikipedia contributors. (2021, May 6). Split-step method.
   In Wikipedia, The Free Encyclopedia.
   https://en.wikipedia.org/wiki/Split-step_method.*/
int schrod_splitstep(Renderer *renderer) {
    int main_frame = renderer->main_frame;
    GLFWwindow *window = renderer->window;
    int exit_status = 0;
    int NX = 256, NY = 256;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    // int NX = 256, NY = 256;
    int frame_count = 0;
    auto imag_unit = std::complex<double>(0.0, 1.0);
    double dt = 3.0;
    // std::complex<double> dt = 3.0 - 0.1*imag_unit;
    double hbar = 1.0;
    double m = 1.0;
    // double dx = width/(float)NX, dy = height/(float)NY;
    double pi = 3.141592653589793;
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    auto command
        = DrawTexture2DData(Path("./shaders/wavepacket/init.frag"));
    command.set_float_uniforms({{"amplitude", 5.0},
                                {"sigmaX", 0.05}, {"sigmaY", 0.05}});
    command.set_vec2_uniforms({{"r0", {.x=0.25, .y=0.25}}, });
    command.set_ivec2_uniforms({{"wavenumber", {.x=-6, .y=5}}, });
    auto psi_unnorm = command.create(COMPLEX, NX, NY, true,
                                     GL_REPEAT, GL_REPEAT,
                                     GL_LINEAR, GL_LINEAR);
    auto psi {psi_unnorm*(100.0/sqrt(psi_unnorm.squared_norm().as_double))};

    /* auto psi_p = 1.0*psi;
    // psi = psi_p;
    init_wavepacket_in_momentum_space(psi_p);
    // psi = psi_p;
    psi = funcs2D::ifft(psi_p);*/

    auto x = funcs2D::make_x(-0.5, 0.5, FLOAT, NX, NY) - 0.5/(double)NX;
    auto y = funcs2D::make_y(-0.5, 0.5, FLOAT, NX, NY) - 0.5/(double)NY;
    Texture2DData pot = 1.0*(0.6*(x*x + y*y)).cast_to(COMPLEX, X, NONE);
    Texture2DData x_propagator = exp((-imag_unit*(dt/2.0)/hbar)*pot);
    Texture2DData px = funcs2D::fftshift(2.0*pi*x).cast_to(COMPLEX, X, NONE);
    Texture2DData py = funcs2D::fftshift(2.0*pi*y).cast_to(COMPLEX, X, NONE);
    Texture2DData p2 = px*px + py*py;
    Texture2DData p_propagator = exp((-imag_unit*dt/(2.0*m*hbar))*p2);
    auto h_psi_func = [&](const Texture2DData psi_x0) -> Texture2DData {
        return x_propagator
        *funcs2D::ifft(p_propagator*funcs2D::fft(x_propagator*psi));
    };
    auto view_com = DrawTexture2DData(Path("./shaders/view.frag"));
    auto data = new uint8_t[3*window_width*window_height] {0,};
    std::time_t t0 = std::time(nullptr);

    int k = 0;
    bool exit_loop = false;
    loop = [&] {
        struct timespec frame_start, frame_end;
        clock_gettime(CLOCK_MONOTONIC, &frame_start);
        glViewport(0, 0, NX, NY);
        int steps_per_frame = 1;
        for (int i = 0; i < steps_per_frame; i++) {
            psi = h_psi_func(psi);
            psi = psi*(100.0/sqrt(psi.squared_norm().as_double));
        }

        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        psi.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();

        /* auto bmp_frame = Texture2DData(HALF_FLOAT3,
                                           window_width, window_height);
        view_com.draw(bmp_frame, "tex", psi);
        bmp_frame.paste_to_rgb_image_data(data);
        if (k % 5 == 0 && k != 0) {
            std::stringstream filename {};
            std::string d_path = "./";
            if (k < 10)
                filename << d_path << "data_0000" << k;
            else if (k < 100)
                filename << d_path << "data_000" << k;
            else if (k < 1000)
                filename << d_path << "data_00" << k;
            else if (k < 10000)
                filename << d_path << "data_0" << k;
            else
                filename << d_path << "data_" << k;
            filename << ".bmp";
            write_to_bitmap(&(filename.str())[0], data,
                             window_width, window_height);
        }*/

        glfwPollEvents();
        frame_count++;
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && k > 30)
            exit_loop = true;
        if (glfwWindowShouldClose(window)) {
            exit_loop = true;
            exit_status = 1;
        }
        glfwSwapBuffers(window);
        clock_gettime(CLOCK_MONOTONIC, &frame_end);
        if (k % 10 == 0 && k != 0) {
            double delta_t = time_difference_in_ms(&frame_start, &frame_end);
	    // std::cout << psi2.sum_reduction().as_dvec4.x << std::endl;
            std::cout << "frames/s: " << (1000.0/delta_t) << std::endl;
            std::cout << "steps/s: " << (1000.0/delta_t)*steps_per_frame
                      << std::endl;
        }
        k++;
    };

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
    #else
    while(!exit_loop)
        loop();
    #endif
    
    std::time_t t1 = std::time(nullptr);
    std::cout << (double)frame_count/((double)t1 - (double)t0) << std::endl;
    return exit_status;
}


static double time_difference_in_ms(const struct timespec *t1,
                                    const struct timespec *t2) {
    if (t2->tv_nsec >= t1->tv_nsec)
        return (double)(t2->tv_nsec - t1->tv_nsec)/1000000.0;
    return (double)(999999999 - t1->tv_nsec + t2->tv_nsec)/1000000.0;
}

