#include <GLES3/gl3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gl_wrappers/gl_wrappers.h"
#include "summation_gl.h"
#include "simulation.h"


struct Click {
    double x, y;
    double dx, dy;
    int pressed;
    int released;
    int w, h;
} left_click;

void click_update(struct Click *click, GLFWwindow *window) {
    double prev_x = click->x;
    double prev_y = click->y;
    glfwGetFramebufferSize(window, &click->w, &click->h);
    glfwGetCursorPos(window, &click->x, &click->y);
    click->x = click->x/(double)click->w;
    click->y = 1.0 - click->y/(double)click->h;
    click->dx = click->x - prev_x;
    click->dy = click->y - prev_y;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        click->pressed = 1;
    } else {
        if (click->released) click->released = 0;
        if (click->pressed) click->released = 1;
        click->pressed = 0;
    }
}

struct SimParams sim_params;
struct Programs programs;
struct Frames quads;

void test_add() {
    for (int i = 0; i < 2; i++) {
        bind_quad(quads.waves[i], programs.init_dist);
        set_vec4_uniform("amplitude", 1.0, 1.0, 1.0, 1.0);
        set_float_uniform("sigma_x", (i == 0)? 0.05: 0.06);
        set_float_uniform("sigma_y", (i == 0)? 0.05: 0.06);
        set_float_uniform("u0", (i == 0)? 0.25: 0.3);
        set_float_uniform("v0", (i == 0)? 0.5: 0.55);
        draw_unbind();
    }
    struct Vec4 s1 = {.x=0.5, .y=0.25, .z=1.0, .w=1.3};
    struct Vec4 s2 = {.x=2.0, .y=1.23, .z=2.0, .w=1.1};
    add(programs.add2, &s1, quads.waves[0], &s2, quads.waves[1],
        quads.waves[2]);
    struct Vec4 *v1 = (struct Vec4 *)malloc(START_WIDTH*START_HEIGHT*
                                            sizeof(struct Vec4));
    struct Vec4 *v2 = (struct Vec4 *)malloc(START_WIDTH*START_HEIGHT*
                                            sizeof(struct Vec4));
    struct Vec4 *v3 = (struct Vec4 *)malloc(START_WIDTH*START_HEIGHT*
                                            sizeof(struct Vec4));
    get_texture_array(quads.waves[0], 0, 0,
                      START_WIDTH, START_HEIGHT, GL_FLOAT, v1);
    get_texture_array(quads.waves[1], 0, 0,
                      START_WIDTH, START_HEIGHT, GL_FLOAT, v2);
    get_texture_array(quads.waves[2], 0, 0,
                      START_WIDTH, START_HEIGHT, GL_FLOAT, v3);
    struct DVec4 res;
    for (int i = 0; i < START_HEIGHT; i++) {
        for (int j = 0; j < START_WIDTH; j++) {
            int index = START_WIDTH*i + j;
            for (int k = 0; k < 4; k++) {
                float sum = s2.ind[k]*v2[index].ind[k]
                    + s1.ind[k]*v1[index].ind[k];
                res.ind[k] += ((double)v3[index].ind[k] - (double)sum);
            }
        }
    }
    printf("%f, %f, %f, %f\n", res.ind[0], res.ind[1],
           res.ind[2], res.ind[3]);
    free(v1);
    free(v2);
    free(v3);
}

void test_subtract() {
    for (int i = 0; i < 2; i++) {
        bind_quad(quads.waves[i], programs.init_dist);
        set_vec4_uniform("amplitude", 1.0, 1.0, 1.0, 1.0);
        set_float_uniform("sigma_x", (i == 0)? 0.05: 0.06);
        set_float_uniform("sigma_y", (i == 0)? 0.05: 0.06);
        set_float_uniform("u0", (i == 0)? 0.25: 0.3);
        set_float_uniform("v0", (i == 0)? 0.5: 0.55);
        draw_unbind();
    }
    struct Vec4 s1 = {.x=0.5, .y=0.25, .z=1.0, .w=1.3};
    struct Vec4 s2 = {.x=2.0, .y=1.23, .z=2.0, .w=1.1};
    subtract(programs.add2, &s1, quads.waves[0], &s2, quads.waves[1],
             quads.waves[2]);
    struct Vec4 *v1 = (struct Vec4 *)malloc(START_WIDTH*START_HEIGHT*
                                            sizeof(struct Vec4));
    struct Vec4 *v2 = (struct Vec4 *)malloc(START_WIDTH*START_HEIGHT*
                                            sizeof(struct Vec4));
    struct Vec4 *v3 = (struct Vec4 *)malloc(START_WIDTH*START_HEIGHT*
                                            sizeof(struct Vec4));
    get_texture_array(quads.waves[0], 0, 0,
                      START_WIDTH, START_HEIGHT, GL_FLOAT, v1);
    get_texture_array(quads.waves[1], 0, 0,
                      START_WIDTH, START_HEIGHT, GL_FLOAT, v2);
    get_texture_array(quads.waves[2], 0, 0,
                      START_WIDTH, START_HEIGHT, GL_FLOAT, v3);
    struct DVec4 res;
    for (int i = 0; i < START_HEIGHT; i++) {
        for (int j = 0; j < START_WIDTH; j++) {
            int index = START_WIDTH*i + j;
            for (int k = 0; k < 4; k++) {
                float s = - s2.ind[k]*v2[index].ind[k]
                          + s1.ind[k]*v1[index].ind[k];
                res.ind[k] += ((double)v3[index].ind[k] - (double)s);
            }
        }
    }
    printf("%f, %f, %f, %f\n", res.ind[0], res.ind[1],
           res.ind[2], res.ind[3]);
    free(v1);
    free(v2);
    free(v3);
}

void test_dot() {
    for (int i = 0; i < 2; i++) {
        bind_quad(quads.waves[i], programs.init_dist);
        set_vec4_uniform("amplitude", 1.0, 1.0, 1.0, 1.0);
        set_float_uniform("sigma_x", (i == 0)? 0.05: 0.06);
        set_float_uniform("sigma_y", (i == 0)? 0.05: 0.06);
        set_float_uniform("u0", (i == 0)? 0.25: 0.3);
        set_float_uniform("v0", (i == 0)? 0.5: 0.55);
        draw_unbind();
    }
    struct Vec4 res = dot(programs.multiply, programs.scale, quads.summations,
                          sim_params.texel_width,
                          quads.waves[0], quads.waves[1], quads.waves[2]);
    struct Vec4 *v1 = (struct Vec4 *)malloc(START_WIDTH*START_HEIGHT*
                                            sizeof(struct Vec4));
    struct Vec4 *v2 = (struct Vec4 *)malloc(START_WIDTH*START_HEIGHT*
                                            sizeof(struct Vec4));
    get_texture_array(quads.waves[0], 0, 0,
                      START_WIDTH, START_HEIGHT, GL_FLOAT, v1);
    get_texture_array(quads.waves[1], 0, 0,
                      START_WIDTH, START_HEIGHT, GL_FLOAT, v2);
    struct DVec4 res2;
    for (int i = 0; i < START_HEIGHT; i++) {
        for (int j = 0; j < START_WIDTH; j++) {
            for (int k = 0; k < 4; k++) {
                res2.ind[k] += ((double)v1[i*START_WIDTH + j].ind[k])*
                                 ((double)v2[i*START_WIDTH + j].ind[k]);
            }
        }
    }
    printf("%f, %f, %f, %f\n", res2.ind[0], res2.ind[1],
           res2.ind[2], res2.ind[3]);
    printf("%f, %f, %f, %f\n", res.ind[0], res.ind[1],
           res.ind[2], res.ind[3]);
    free(v1);
    free(v2);
}

int main() {
    int width = START_WIDTH, height = START_HEIGHT;
    #ifndef __APPLE__
    int view_ratio = 512/width;
    #else
    int view_ratio = 1024/width;
    #endif
    int pixel_width = view_ratio*START_WIDTH;
    int pixel_height = view_ratio*START_HEIGHT;
    GLFWwindow *window = init_window(pixel_width, pixel_height);

    init_sim_params(&sim_params);
    init_programs(&programs);
    init_frames(&quads, &sim_params);

    glViewport(0, 0, width, height);

    test_dot();

    for (int i = 0; i < 3; i++) {
        bind_quad(quads.waves[i], programs.init_dist);
        set_vec4_uniform("amplitude", 1.0, 1.0, 1.0, 1.0);
        set_float_uniform("sigma_x", 0.01);
        set_float_uniform("sigma_y", 0.01);
        set_float_uniform("u0", 0.5);
        set_float_uniform("v0", 0.5);
        draw_unbind();
    }
    bind_quad(quads.density, programs.init_dist);
    set_vec4_uniform("amplitude", 1.0, 5.0, 3.0, 5.0);
    set_float_uniform("sigma_x", 0.5);
    set_float_uniform("sigma_y", 0.5);
    set_float_uniform("u0", 0.5);
    set_float_uniform("v0", 0.5);
    draw_unbind();
    bind_quad(quads.dissipator, programs.init_dist);
    set_vec4_uniform("amplitude", 1.0, 1.0, 1.0, 1.0);
    set_float_uniform("sigma_x", 0.07);
    set_float_uniform("sigma_y", 0.07);
    set_float_uniform("u0", 0.25);
    set_float_uniform("v0", 0.25);
    draw_unbind();

    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        glViewport(0, 0, width, height);
        if (left_click.pressed && k >= 0) {
            // TODO
        } else {
            // TODO
        }

        timestep(&sim_params, &programs, &quads);
        sim_params.t += sim_params.dt;

        glViewport(0, 0, pixel_width, pixel_height);
        bind_quad(quads.main_view, programs.view);
        set_sampler2D_uniform("tex", quads.waves[1]);
        draw_unbind();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}
