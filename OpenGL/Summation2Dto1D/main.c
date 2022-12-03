#include <GLES3/gl3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gl_wrappers.h"
#include "gl_wrappers/gl_wrappers.h"

const int PIXEL_WIDTH = 512;
const int PIXEL_HEIGHT = 512;

struct Vec4 arr2D_4x4[PIXEL_WIDTH*PIXEL_HEIGHT];
struct Vec4 arr1D_4x4[PIXEL_HEIGHT];
struct Vec4 arr1D_4x4_tex_res[PIXEL_HEIGHT] = {{0.0,},};
struct DVec4 arr2D_4x8[PIXEL_WIDTH*PIXEL_HEIGHT];
struct DVec4 arr1D_4x8[PIXEL_HEIGHT];

void reduce_2D_to_1D_vec4(struct Vec4 *arr1d, struct Vec4 *arr2d, int width,
                          int height) {
    for (int i = 0; i < height ; i++) {
        for (int k = 0; k < 4; k++) {
            float sum = 0.0;
            for (int j = 0; j < width; j++) {
                sum += (float)arr2d[width*i + j].ind[k];
            }
            arr1d[i].ind[k] = sum;
        }
    }
}

void reduce_2D_to_1D_dvec4(struct DVec4 *arr1d, struct DVec4 *arr2d, int width,
                           int height) {
    for (int i = 0; i < height ; i++) {
        for (int k = 0; k < 4; k++) {
            double sum = 0.0;
            for (int j = 0; j < width; j++) {
                sum += (double)arr2d[width*i + j].ind[k];
            }
            arr1d[i].ind[k] = sum;
        }
    }
}

void reduce_texture2D_to_texture1D(int scale_program, frame_id init_quad,
                                   frame_id *sum_quads, size_t init_size) {
    int i = 0;
    for (int size=init_size/2; size >= 1; i++, size /= 2) {
        glViewport(0, 0, size, init_size);
        bind_quad(sum_quads[i], scale_program);
        set_float_uniform("scale", 2.0);
        frame_id tmp = (i == 0)? init_quad: sum_quads[i-1];
        set_sampler2D_uniform("tex", tmp);
        draw_unbind();
    }
    glViewport(0, 0, init_size, init_size);
}

int main() {
    int width = PIXEL_WIDTH, height = PIXEL_HEIGHT;

    // Initialize the window
    GLFWwindow *window = init_window(width, height);

    // Make the shader programs
    // view_program = make_program("./shaders/copy.frag");
    int zero_program = make_program("./shaders/zero.frag");
    int copy_program = make_program("./shaders/copy.frag");
    int scale_program = make_program("./shaders/scale.frag");

    struct TextureParams texture_params = {
        .type=GL_FLOAT,
        .width=width, .height=height,
        .generate_mipmap=1,
        .wrap_s=GL_CLAMP_TO_EDGE, .wrap_t=GL_CLAMP_TO_EDGE,
        .min_filter=GL_LINEAR, .mag_filter=GL_LINEAR
    };
    frame_id view_quad = new_quad(NULL);
    frame_id draw_quad = new_quad(&texture_params);
    int sum_quads_len = (int)(log(width)/log(2));
    frame_id *sum_quads = malloc(sizeof(frame_id)*sum_quads_len);
    for (int i = 0; i < sum_quads_len; i++) {
        texture_params.width /= 2;
        sum_quads[i] = new_quad(&texture_params);
    }

    // Initialize the draw quad to zero.
    bind_quad(draw_quad, zero_program);
    draw_unbind();

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float u = j/(float)width, v = i/(float)height;
            float sx = 0.07, sy = 0.07;
            float val = exp(-0.5*powf(u - 0.1, 2)/(sx*sx)
                            -0.5*powf(v - 0.1, 2)/(sy*sy));
            float pi = 3.14159;
            float n_u = 3.0;
            val *= powf(cos(2.0*pi*u*n_u), 2.0);
            int index = i*width + j;
            arr2D_4x4[index].ind[0] = val;
            arr2D_4x4[index].ind[1] = val;
            arr2D_4x4[index].ind[2] = val;
            arr2D_4x4[index].ind[3] = 1.0;
            arr2D_4x8[index].ind[0] = (double)val;
            arr2D_4x8[index].ind[1] = (double)val;
            arr2D_4x8[index].ind[2] = (double)val;
            arr2D_4x8[index].ind[3] = (double)1.0;
        }
    }
    substitute_array(draw_quad, width, height, GL_FLOAT, arr2D_4x4);
    reduce_texture2D_to_texture1D(scale_program, draw_quad,
                                  sum_quads, width);
    get_texture_array(sum_quads[sum_quads_len-1], 0, 0, 1, height, GL_FLOAT,
                      arr1D_4x4_tex_res);
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        bind_quad(view_quad, copy_program);
        set_sampler2D_uniform("tex", draw_quad);
        draw_unbind();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    reduce_2D_to_1D_vec4(arr1D_4x4, arr2D_4x4, width, height);
    reduce_2D_to_1D_dvec4(arr1D_4x8, arr2D_4x8, width, height);
    for (int i = 0; i < height; i++) {
        if (fabs(arr1D_4x4[i].r) > 30.0)
            printf("%d, r, %f, %f, %f\n", i,
                   arr1D_4x4[i].r, arr1D_4x8[i].r, arr1D_4x4_tex_res[i].r);
        /*if (fabs(arr1D_4x4[i].g) > 30.0)
            printf("%d, g, %f, %f\n", i, arr1D_4x4[i].g, arr1D_4x8[i].g);
        if (fabs(arr1D_4x4[i].b) > 30.0)
        printf("%d, b, %f, %f\n", i, arr1D_4x4[i].b, arr1D_4x8[i].b);*/

    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;

}
