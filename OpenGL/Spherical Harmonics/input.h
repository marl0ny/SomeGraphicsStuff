#include "gl_wrappers/gl_wrappers.h"
#include <GLFW/glfw3.h>
#include <cmath>

#ifndef __INPUT__
#define __INPUT__

struct Input {
    virtual void update(GLFWwindow *window) = 0;
    // virtual ~Input() {};
};


struct CharacterInput: Input {
    int keycode = 0;
    bool pressed = false;
    bool released = false;
    CharacterInput(int k): keycode(k) {};
    void update(GLFWwindow *window) override {
        if (glfwGetKey(window, keycode) == GLFW_PRESS) {
            pressed = true;
        } else {
            if (released) released = false;
            if (pressed) released = true;
            pressed = false;
        }
    }
};

struct Mouse: Input {
    double x = 0.0; 
    double  y = 0.0;
    double dx = 0.0;
    double dy = 0.0;
    bool pressed = false;
    bool released = false;
    int w=0, h=0;
    int type = GLFW_MOUSE_BUTTON_1;

    Mouse(int type) {
        this->type = type;
    }

    double get_delta2() const {
        return dx*dx + dy*dy;
    }

    double get_delta() const {
        return sqrt(get_delta2());
    }

    void update(GLFWwindow *window) override {
        double x_prev = x;
        double y_prev = y;
        glfwGetFramebufferSize(window, &w, &h);
        glfwGetCursorPos(window, &x, &y);
        x = x/(double)w;
        y = 1.0 - y/(double)h;
        dx = x - x_prev;
        dy = y - y_prev;
        if (glfwGetMouseButton(window, type) == GLFW_PRESS) {
            pressed = true;
        } else {
            if (released) released = false;
            if (pressed) released = true;
            pressed = false;
        }
    }
};

#endif
