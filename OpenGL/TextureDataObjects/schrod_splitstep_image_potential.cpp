#include "schrod_splitstep_image_potential.hpp"

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
#include <cstdlib>
#include "write_to_png.h"
#include "read_from_png.h"

static std::vector<Vec4> get_potential_from_png_file(std::string path,
                                                     int width, int height) {
    unsigned char *raw = (unsigned char *)malloc(width*height*3);
    read_png((char *)path.c_str(), raw, width*height*3);
    auto data = std::vector<Vec4>(width*height);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            data[i*width + j].r = (float)raw[3*(i*width + j)];
            data[i*width + j].g = (float)raw[3*(i*width + j) + 1];
            data[i*width + j].b = (float)raw[3*(i*width + j) + 2];
            data[i*width + j].a = 0.0;
        }
    }
    free(raw);
    return data;
}

static std::vector<Vec4> get_potential_from_bmp_file(std::string path) {
    int width = 0, height = 0;
    unsigned char *raw
        = get_bitmap_contents((char *)path.c_str(), &width, &height);
    if (raw == NULL) {
        std::cerr << "Unable to open " << path << std::endl;
        return {};
    }
    auto data = std::vector<Vec4>(width*height);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            data[i*width + j].r = (float)raw[3*(i*width + j)];
            data[i*width + j].g = (float)raw[3*(i*width + j) + 1];
            data[i*width + j].b = (float)raw[3*(i*width + j) + 2];
            data[i*width + j].a = 0.0;
        }
    }
    return data;
}

/* Split operator implementation for the Schrodinger equation.

References:

 - James Schloss. The Split-Operator Method. In The Arcane Algorithm Archive.
   https://www.algorithm-archive.org/contents/
   split-operator_method/split-operator_method.html.

 - Wikipedia contributors. (2021, May 6). Split-step method.
   In Wikipedia, The Free Encyclopedia.
   https://en.wikipedia.org/wiki/Split-step_method.

*/
int schrod_splitstep_image_potential(GLFWwindow *window,frame_id main_frame) {
    int exit_status = 0;
    // int NX = 2048, NY = 1024;
    int NX = 1024, NY = 512;
    // int NX = 512, NY = 256;
    // int NX = 256, NY = 128;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    // int NX = 256, NY = 256;
    int frame_count = 0;
    float phase_adjust_scale = 0.09;
    auto imag_unit = std::complex<double>(0.0, 1.0);
    // double dt = 1.0;
    // std::complex<double> dt = 3.0 - 0.1*imag_unit;
    double r_time = 0.0;
    double hbar = 1.0;
    double m = 1.0;
    bool use_nonlinear = false;
    // double dx = width/(float)NX, dy = height/(float)NY;
    double pi = 3.141592653589793;
    int view_program = make_quad_program("./shaders/view.frag");
    glViewport(0, 0, NX, NY);
    std::string path = "";
    path = std::string(std::getenv("HOME")) + "/Pictures/image.png";
    std::cout << path << std::endl;
    /* std::cout << "Enter path: ";
    std::cin >> path;
    std::cout << std::endl;*/
    // auto potential_data
    //     = get_potential_from_bmp_file(path);
    auto potential_data
        = get_potential_from_png_file(path, NX, NY);
    auto potential_image_tex
        = (0.45/255.0)*Texture2DData(&potential_data[0], NX, NY);
    auto command
        = DrawTexture2DData(Path("./shaders/init-gaussian.frag"));
    command.set_float_uniforms({{"amplitude", 5.0},
                                {"sigmaX", 0.05}, {"sigmaY", 0.1}});
    command.set_vec2_uniforms({{"r0", {.x=0.25, .y=0.35}}, });
    if (NY <= 128)
        command.set_ivec2_uniforms({{"wavenumber", {.x=20, .y=10}}, });
    else
        command.set_ivec2_uniforms({{"wavenumber", {.x=40, .y=20}}, });
    auto psi = command.create(COMPLEX, NX, NY, true,
                               GL_REPEAT, GL_REPEAT,
                               GL_LINEAR, GL_LINEAR);
    // auto psi = Texture2DData(COMPLEX, "./file.dat");
    auto psi1 = psi*(500.0/sqrt(psi.squared_norm().as_double));
    auto psi2 = psi1;
    auto x = make_x(-0.5, 0.5, FLOAT, NX, NY) - 0.5/(double)NX;
    auto y = make_y(-0.5, 0.5, FLOAT, NX, NY) - 0.5/(double)NY;
    auto px = fftshift(2.0*pi*x).cast_to(COMPLEX, X, NONE);
    auto py = fftshift(2.0*pi*y).cast_to(COMPLEX, X, NONE);

    auto pot = potential_image_tex.cast_to(COMPLEX, X, NONE);
    // auto pot = (0.6*(x*x + y*y)).cast_to(COMPLEX, X, NONE);
    // auto pot = zeroes(COMPLEX, NX, NY);

    std::complex<double> dt = 1.0;
    // std::complex<double> zdt = 3.0 - 0.1*imag_unit;
    std::complex<double> zdt = 1.0 - 0.0333*imag_unit;
    if (NY <= 128) {
        dt *= 0.25;
        zdt *= 0.25;
    }
    auto p_propagator = exp((-imag_unit*dt/(2.0*m*hbar))*(px*px + py*py));
    auto p_propagator_complex
         = exp((-imag_unit*zdt/(2.0*m*hbar))*(px*px + py*py));
    auto x_propagator = exp((-imag_unit*(dt/2.0)/hbar)*pot);
    auto h_psi_func = [=](Texture2DData &psi,
                          Texture2DData &pot) -> Texture2DData {
        return x_propagator*ifft(p_propagator*fft(x_propagator*psi));
    };
    double nl_coeff = 0.1;
    if (NY <= 128) nl_coeff = 0.001;
    auto x_propagator_nonlinear = [=](Texture2DData &psi,
                                         Texture2DData &pot) -> Texture2DData {
        // double nl_scale = 10.0*(double)NY/512.0;
        return exp((-imag_unit*(zdt/2.0)/hbar)*(pot + nl_coeff*psi*conj(psi)));
    };
    auto h_psi_func_nonlinear = [&](Texture2DData &psi,
                          Texture2DData &pot) -> Texture2DData {
        Texture2DData psi2 = ifft(p_propagator_complex
                                  *fft(x_propagator_nonlinear(psi, pot)*psi));
        return x_propagator_nonlinear(psi2, pot)*psi2;
    };
    auto view_com = DrawTexture2DData(Path("./shaders/view.frag"));
    view_com.set_float_uniforms({{"phaseAdjust", phase_adjust_scale*r_time}});
    auto data = new uint8_t[3*window_width*window_height] {0,};
    std::time_t t0 = std::time(nullptr);
    for (int k = 0, exit_loop = false; !exit_loop; k++) {
        int steps_frame = 16;
        glViewport(0, 0, NX, NY);
        for (int i = 0; i < steps_frame; i++) {
            if (use_nonlinear)
                psi2 = h_psi_func_nonlinear(psi1, pot);
            else
                psi2 = h_psi_func(psi1, pot);
            double norm_val = sqrt(psi2.squared_norm().as_double);
            psi2 = psi2*(500.0/norm_val);
            r_time += sqrt(dt.real()*dt.real() + dt.imag()*dt.imag());
            swap(psi1, psi2);
        }
        glViewport(0, 0, window_width, window_height);
        /*{
            auto bmp_frame = Texture2DData(HALF_FLOAT3,
                                           window_width, window_height);
            view_com.draw(bmp_frame, "tex", psi1);

            bmp_frame.paste_to_rgb_image_data(data);
            std::stringstream filename {};
            std::string d_path = "/Volumes/T7/bmp_files7/";
            // std::string d_path = "./";
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
            
            // filename << ".png";
            // write_rgb8_png(&(filename.str())[0], 
            //                data, window_width, window_height);

            filename << ".bmp";
            write_to_bitmap(&(filename.str())[0], data,
                            window_width, window_height);
            
        }*/
        bind_quad(main_frame, view_program);
        psi1.set_as_sampler2D_uniform("tex");
        // std::cout << r_time << std::endl;
        set_float_uniform("phaseAdjust", phase_adjust_scale*r_time);
        // potential_image_tex.set_as_sampler2D_uniform("bgTex");
        draw_unbind_quad();
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            use_nonlinear = true;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            std::cout << phase_adjust_scale << "\n";
            phase_adjust_scale += 0.01;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            std::cout << phase_adjust_scale << "\n";
            phase_adjust_scale -= 0.01;
        }
        /* if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {

            psi2.serialize(std::string("file.dat"));
        }*/
        frame_count++;
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && k > 30)
            exit_loop = true;
        if (glfwWindowShouldClose(window)) {
            psi1.serialize(std::string("file.dat"));
            exit_loop = true;
            exit_status = 1;
        }
        glfwSwapBuffers(window);
    }
    std::time_t t1 = std::time(nullptr);
    std::cout << (double)frame_count/((double)t1 - (double)t0) << std::endl;
    return exit_status;
}

