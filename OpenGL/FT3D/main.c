#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gl_wrappers/gl_wrappers.h"
#include "init_render.h"


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
    #ifdef __APPLE__
    click->x = 2.0*click->x/(double)click->w;
    click->y = 1.0 - 2.0*click->y/(double)click->h;
    #else
    click->x = click->x/(double)click->w;
    click->y = 1.0 - click->y/(double)click->h;
    #endif
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

static double s_scroll = 0.1;
void scroll_callback(GLFWwindow *window, double x, double y) {
    s_scroll += (2.0*y)/25.0;
}


int main() {
    #ifdef __APPLE__
    int pixel_width = 1024;
    int pixel_height = 1024;
    #else
    int pixel_width = 512;
    int pixel_height = 512;
    #endif
    GLFWwindow *window = init_window(pixel_width, pixel_height);
    init();
    struct RenderParams render_params = {};
    glfwSetScrollCallback(window, scroll_callback);
    render_params.move_x = 50;
    render_params.move_y = 50;
    render_params.move_z = 50;
    render_params.inc_mode1 = 0;
    render_params.inc_mode2 = 100;
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        if (left_click.pressed) {
            render_params.user_use = 1;
            render_params.x = left_click.x;
            render_params.y = left_click.y;
            render_params.dx = -left_click.dx;
            render_params.dy = -left_click.dy;
        } else {
            render_params.user_use = 0;
        }
        render_params.scroll = s_scroll;
        render(&render_params);
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
            render_params.view_mode = 0;
        }
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            render_params.view_mode = 1;
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            render_params.view_mode = 2;
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            render_params.move_y += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            render_params.move_y -= 1;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            render_params.move_x -= 1;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            render_params.move_x += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            render_params.move_z += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            render_params.move_z -= 1;
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            render_params.inc_mode1 += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            render_params.inc_mode1 -= 1;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            render_params.inc_mode2 += 10;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            render_params.inc_mode2 -= 10;
        }
        render_params.move_x %= 100;
        render_params.move_y %= 100;
        render_params.move_z %= 100;
        click_update(&left_click, window);
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}