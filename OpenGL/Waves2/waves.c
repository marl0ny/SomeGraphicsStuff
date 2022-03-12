#include <stdio.h>
#include <stdlib.h>
#include "gl_wrappers.h"


struct Click {
    double x, y;
    int pressed;
    int released;
    int w, h;
};

void click_update(struct Click *click, GLFWwindow *window) {
    glfwGetFramebufferSize(window, &click->w, &click->h);
    glfwGetCursorPos(window, &click->x, &click->y);
    click->x = 2.0*click->x/(double)click->w;
    click->y = 1.0 - 2.0*click->y/(double)click->h;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        click->pressed = 1;
    } else {
        if (click->released) click->released = 0;
        if (click->pressed) click->released = 1;
        click->pressed = 0;
    }
}

int main() {
    int width = 1024, height = 1024;
    int view_ratio = 1;
    int pixel_width = view_ratio*width;
    int pixel_height = view_ratio*height;
    GLFWwindow *window = init_window(pixel_width, pixel_height);
    // init_glew();
    GLuint vert_shader = get_shader("./shaders/vertices.vert", GL_VERTEX_SHADER);
    GLuint view_shader = get_shader("./shaders/view.frag", GL_FRAGMENT_SHADER);
    GLuint make_shader = get_shader("./shaders/make-wave.frag", GL_FRAGMENT_SHADER);
    GLuint step_shader = get_shader("./shaders/wave-step.frag", GL_FRAGMENT_SHADER);
    GLuint view_program = make_program(vert_shader, view_shader);
    GLuint draw_program = make_program(vert_shader, make_shader);
    GLuint step_program = make_program(vert_shader, step_shader);
    glDeleteShader(vert_shader);
    glDeleteShader(view_shader);
    glDeleteShader(make_shader);
    glDeleteShader(step_shader);
    int view_frame = new_ubyte_quad(pixel_width, pixel_height);
    int draw_frame = new_float_quad(width, height);
    int f1 = new_float_quad(width, height);
    int f2 = new_float_quad(width, height);
    int f3 = new_float_quad(width, height);
    int quads[3] = {f1, f2, f3};
    struct Click left_click;
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        if (left_click.released && k > 20) {
            for (int i = 0; i < 2; i++) {
                // printf("pressed\n");
                // printf("%g, %g\n", left_click.x, left_click.y);
                bind_quad(draw_frame, draw_program);
                set_float_uniform("r", 1.0*(k%3 == 0));
                set_float_uniform("g", 1.0*(k%3 == 1));
                set_float_uniform("b", 1.0*(k%3 == 2));
                set_float_uniform("sigma", 0.01);
                set_float_uniform("x0", left_click.x);
                set_float_uniform("y0", left_click.y);
                set_float_uniform("a", 0.2);
                set_int_uniform("tex", quads[i]);
                draw();
                unbind();
                bind_quad(quads[i], view_program);
                set_int_uniform("tex", draw_frame);
                draw();
                unbind();
            }
        }
        bind_quad(quads[2], step_program);
        set_float_uniform("dx", 1.0/width);
        set_float_uniform("dy", 1.0/height);
        set_int_uniform("tex1", quads[0]);
        set_int_uniform("tex2", quads[1]);
        draw();
        unbind();
        // glViewport(0, 0, pixel_width, pixel_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        bind_quad(view_frame, view_program);
        set_int_uniform("tex", quads[2]);
        draw();
        unbind();
        // printf("%d\n", glfwGetError(NULL));
        glfwPollEvents();
        click_update(&left_click, window);
        // glViewport(0, 0, width, height);
        glfwSwapBuffers(window);
        int tmp = quads[0];
        quads[0] = quads[1];
        quads[1] = quads[2];
        quads[2] = tmp;
    }
    glDeleteProgram(view_program);
    glDeleteProgram(draw_program);
    glDeleteProgram(step_program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}