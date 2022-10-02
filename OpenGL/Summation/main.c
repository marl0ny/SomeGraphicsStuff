#include <GLES3/gl3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gl_wrappers.h"
#include "gl_wrappers/gl_wrappers.h"


struct Vec4 sum_vec4_array(int width, int height,
                           const struct Vec4 *arr) {
    struct Vec4 res;
    for (int k = 0; k < 4; k++) {
        res.ind[k] = 0.0;
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            for (int k = 0; k < 4; k++) {
                res.ind[k] += arr[i*width + j].ind[k];
            }
        }
    }
    return res;
}

struct Vec4 texture_reduction_sum(int scale_program,
                                  frame_id init_quad, frame_id *sum_quads,
                                  size_t init_size) {
    int i = 0;
    for (int size = init_size/2; size >= 2; i++, size /= 2) {
        glViewport(0, 0, size, size);
        bind_quad(sum_quads[i], scale_program);
        set_float_uniform("scale", 4.0);
        frame_id tmp = (i == 0)? init_quad: sum_quads[i-1];
        set_sampler2D_uniform("tex", tmp);
        draw_unbind();
    }
    struct Vec4 arr[4];
    get_texture_array(sum_quads[i-1], 0, 0, 2, 2, GL_FLOAT, arr);
    glViewport(0, 0, init_size, init_size);
    return sum_vec4_array(2, 2, arr);
}

struct DVec4 sum_dvec4_array(int width, int height,
                             const struct DVec4 *arr) {
    struct DVec4 res;
    for (int k = 0; k < 4; k++) {
        res.ind[k] = 0.0;
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            for (int k = 0; k < 4; k++) {
                res.ind[k] += arr[i*width + j].ind[k];
            }
        }
    }
    return res;
}

struct Vec4 buf1[512*512];
struct Vec4 buf2[512*512];
struct Vec4 buf3[512*512];
struct DVec4 buf4[512*512];


int main() {
    int width = 512, height = 512;

    // Initialize the window
    GLFWwindow *window = init_window(width, height);

    // Make the shader programs
    // view_program = make_program("./shaders/copy.frag");
    int zero_program = make_program("./shaders/zero.frag");
    int copy_program = make_program("./shaders/copy.frag");
    int scale_program = make_program("./shaders/scale.frag");

    // Create the textures that we will use
    // for drawing
    struct TextureParams texture_params = {
        .type=GL_FLOAT,
        .width=width, .height=height,
        .generate_mipmap=1,
        .wrap_s=GL_CLAMP_TO_EDGE, .wrap_t=GL_CLAMP_TO_EDGE,
        .min_filter=GL_LINEAR,
        .mag_filter=GL_NEAREST_MIPMAP_NEAREST,
    };
    frame_id view_quad = new_quad(NULL);
    frame_id draw_quad = new_quad(&texture_params);
    frame_id sum_quads[9];
    // texture_params.generate_mipmap = 0;
    for (int i = 0; i < 8; i++) {
        texture_params.width /= 2;
        texture_params.height /= 2;
        sum_quads[i] = new_quad(&texture_params);
    }

    // Initialize the draw quad to zero.
    bind_quad(draw_quad, zero_program);
    draw_unbind();

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float y = i/(float)height;
            float x = j/(float)width;
            float sx=0.05, sy = 0.07;
            float val = exp(-0.5*powf(x-0.5, 2)/(sx*sx)
                                 -0.5*powf(y-0.5, 2)/(sy*sy));
            // float tau = 2.0*3.14159;
            // float val = fabs(sin(tau*3*x)*cos(tau*5*y));
            // float val = (x-0.25)*y + x*x + 2.0*y*y;
            buf1[i*width + j].r = 100.0*val;
            buf1[i*width + j].g = 0.0;
            buf1[i*width + j].b = 0.0;
            buf1[i*width + j].a = 1.0;
            buf4[i*width + j].r = 100.0*(double)val;
            buf4[i*width + j].g = 0.0;
            buf4[i*width + j].b = 0.0;
            buf4[i*width + j].a = 1.0;
        }
    }
    substitute_array(draw_quad, width, height, GL_FLOAT, buf1);

    /* for (int i = 0, size = 256; i < 8; i++, size /= 2) {
        // fprintf(stdout, "%d\n", size);
        glViewport(0, 0, size, size);
        bind_quad(sum_quads[i], scale_program);
        set_float_uniform("scale", 4.0);
        frame_id tmp = (i == 0)? draw_quad: sum_quads[i-1];
        set_sampler2D_uniform("tex", tmp);
        draw_unbind();
    }
    int dimensions[9] = {256, 128, 64, 32, 16, 8, 4, 2, 1};
    int index = 7;
    int d = dimensions[index];
    glViewport(0, 0, d, d);
    get_texture_array(sum_quads[index], 0, 0, d, d, GL_FLOAT, buf2);
    struct Vec4 res2 = sum_vec4_array(d, d, buf2);
    glViewport(0, 0, width, height);*/
    struct Vec4 res2 = texture_reduction_sum(scale_program,
                                             draw_quad, sum_quads, 512);
    get_texture_array(draw_quad, 0, 0, width, height, GL_FLOAT, buf3);

    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        bind_quad(view_quad, copy_program);
        set_sampler2D_uniform("tex", draw_quad);
        draw_unbind();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    struct Vec4 res1 = sum_vec4_array(width, height, buf1);
    struct Vec4 res3 = sum_vec4_array(width, height, buf3);
    struct DVec4 res4 = sum_dvec4_array(width, height, buf4);
    puts("\tSummation");
    printf("initial float array                 : %f\n", res1.ind[0]);
    printf("initial float texture back to array : %f\n", res3.ind[0]);
    printf("initial double array                : %f\n", res4.ind[0]);
    printf("texture interpolation reductions    : %f\n", res2.ind[0]);
    return 0;
}

