#include "interactor.hpp"
#include <cmath>

double Interactor::scroll = 25.0/2.0;

Interactor::Interactor(GLFWwindow *window) {
    Interactor::attach_scroll_callback(window);
}

void Interactor::click_update(Renderer *renderer) {
    GLFWwindow *window = renderer->window;
    double prev_x = this->left_click.x;
    double prev_y = this->left_click.y;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glfwGetCursorPos(window, &this->left_click.x, &this->left_click.y);
    #ifdef __APPLE__
    this->left_click.x = 2.0*this->left_click.x/(double)width;
    this->left_click.y = 1.0 - 2.0*this->left_click.y/(double)height;
    #else
    this->left_click.x = this->left_click.x/(double)width;
    this->left_click.y = 1.0 - this->left_click.y/(double)height;
    #endif
    this->left_click.dx = this->left_click.x - prev_x;
    this->left_click.dy = this->left_click.y - prev_y;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        this->left_click.pressed = 1;
    } else {
        if (this->left_click.released) this->left_click.released = 0;
        if (this->left_click.pressed) this->left_click.released = 1;
        this->left_click.pressed = 0;
    }
}


void Interactor::scroll_callback(GLFWwindow *window, double x, double y) {
    // std::cout << "Value of y: " << y << std::endl;
    Interactor::scroll += y;
    if (Interactor::scroll < 0)
        Interactor::scroll = 0.0;
}

void Interactor::attach_scroll_callback(GLFWwindow *window) {
    glfwSetScrollCallback(window, Interactor::scroll_callback);
}


DVec2 Interactor::get_mouse_position() {
    return {.x=this->left_click.x, .y=this->left_click.y};
}

DVec2 Interactor::get_mouse_delta() {
    return {.x=this->left_click.dx, .y=this->left_click.dy};
}

double Interactor::get_mouse_abs_delta() {
    return sqrt(this->left_click.dx*this->left_click.dx 
                + this->left_click.dy*this->left_click.dy);
}


double Interactor::get_scroll() {
    return Interactor::scroll;
}

bool Interactor::left_pressed() {
    return left_click.pressed;
}

bool Interactor::left_released() {
    return left_click.released;
}

bool Interactor::middle_pressed() {
    return middle_click.pressed;
}

bool Interactor::right_pressed() {
    return right_click.pressed;
}