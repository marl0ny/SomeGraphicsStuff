#include "reduction_to_1d.hpp"

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


/* Reduce a 2D texture to 1D by summing over either the rows or columns.
*/
int reduction_to_1d(GLFWwindow *window,
                    frame_id main_frame) {
    int exit_status = 0;
    int window_width = 0, window_height = 0;
    window_dimensions(window, &window_width, &window_height);
    int NX = window_width, NY = window_height;
    int view_program = make_quad_program("./shaders/copy.frag");
    auto *data = new Vec4[NX*NY] {0.,};
    struct FillDataRet {
        DVec4 elementwise_red_1d[0x1000];
        double norm_squared_1d[0x1000];
        DVec4 elementwise_sum;
        double norm_squared;
    };
    auto fill_data = [&]() -> FillDataRet {
        struct FillDataRet res = {{0,}, {0,}, 
                0, 0};
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
                res.elementwise_sum.x += x;
                res.elementwise_sum.y += y;
                res.elementwise_sum.z += z;
                res.elementwise_sum.w += w;
                res.elementwise_red_1d[i].x += x;
                res.elementwise_red_1d[i].y += y;
                res.elementwise_red_1d[i].z += z;
                res.elementwise_red_1d[i].w += w;
                // float xf = (float)x, yf = (float)y, zf = (float)z, wf = (float)w;
                res.norm_squared += x*x + y*y + z*z + w*w;
                res.norm_squared_1d[i] += x*x + y*y + z*z + w*w;
            }
        }
        return res;
    };
    Uint8Vec4 *byte4_arr = (Uint8Vec4 *)calloc(window_width*window_height, 
                                          sizeof(Uint8Vec4));
    for (int k = 0, exit_loop=false; !exit_loop; k++) {
        FillDataRet tmp = fill_data();
        float tmp2[0x1000];
        Texture2DData tmp_tex{data, window_width, window_height};
        auto tmp_tex_squared = tmp_tex*tmp_tex;
        auto tmp_tex_squared_sc = tmp_tex_squared.reduce_to_single_channel();
        Texture2DData data_red = tmp_tex_squared_sc.reduce_to_column();
        data_red.paste_to_array((float *)tmp2);
        double max_val = 0.0;
        double max_val2 = 0.0;
        for (int i = 0; i < window_height*window_width; i++) {
            byte4_arr[i].r = 0;
            byte4_arr[i].g = 0;
            byte4_arr[i].b = 0;
            byte4_arr[i].a = 0;
        }
        for (int i = 0; i < NY; i++)
            max_val = (tmp.norm_squared_1d[i] > max_val)?
                tmp.norm_squared_1d[i]: max_val;
        for (int i = 0; i < NY; i++)
            max_val2 = (tmp2[i] > max_val2)? tmp2[i]: max_val2;
        printf("Max val using cpu vs glsl\n");
        printf("%g, %g\n", max_val, max_val2);
        for (int i = 0; i < NY; i++) {
            int index0 = i;
            int index1 = (int)(0.5*NX*tmp.norm_squared_1d[i]/max_val);
            int index2 = (int)(0.5*NX*tmp2[i]/max_val2);
            if (index1 >= NX) index1 = NY - 1;
            if (index1 < 0) index1 = 0;
            if (index2 >= NX) index2 = NY - 1;
            if (index2 < 0) index2 = 0;
            // int index1 = 100;
            byte4_arr[index0*NY + index1].x = 127;
            byte4_arr[index0*NY + index1].y = 0;
            byte4_arr[index0*NY + index2].z = 127;
            byte4_arr[index0*NY + index1].a = 127;
            byte4_arr[index0*NY + index2].a = 127;
        }
        Texture2DData t{byte4_arr, window_width, window_height};
        // Texture2DData t2{byte4_arr2, window_width, window_height};
        bind_quad(main_frame, view_program);
        // data_red.set_as_sampler2D_uniform("tex");
        t.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        /*FillDataRet tmp = fill_data();
        DVec4 sum = tmp.elementwise_sum;
        double norm_squared_sum = tmp.norm_squared;
        std::cout << "Array sum for loop:" << std::endl;
        std::cout << "x: " << sum.x << ", y: " << sum.y
         << ", z: " << sum.z << ", w: " << sum.w << std::endl;
        auto f = Texture2DData(data, NX, NY);
        std::cout << "Using method: " << std::endl;
        PixelData sum2 = f.sum_reduction();
        std::cout << "x: " << sum2.as_dvec4.x << ", y: " << sum2.as_dvec4.y
         << ", z: " << sum2.as_dvec4.z << ", w: " << sum2.as_dvec4.w
         << std::endl;
        std::cout << "Norm squared for loop: ";
        std::cout << norm_squared_sum << std::endl;
        std::cout << "Using method: ";
        double tmp2 = f.squared_norm().as_double;
        std::cout << tmp2 << std::endl;
        bind_quad(main_frame, view_program);
        f.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();*/
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
