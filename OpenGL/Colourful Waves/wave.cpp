#include "gl_wrappers.h"

struct {
    double x, y;
    bool pressed = false;
    bool released = false;
    int w, h;
    void update(GLFWwindow *window) {
        glfwGetFramebufferSize(window, &w, &h);
        glfwGetCursorPos(window, &x, &y);
        x = x/(double)w;
        y = 1.0 - y/(double)h;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
            pressed = true;
        } else {
            if (released) released = false;
            if (pressed) released = true;
            pressed = false;
        }
    }
} left_click;

int main() {
    int width = 720, height = 720;
    GLFWwindow *window = init_window(width, height);
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
    Quad view_frame = Quad::make_frame(width, height);
    Quad draw_frame = Quad::make_float_frame(width, height);
    Quad f1 = Quad::make_float_frame(width, height);
    Quad f2 = Quad::make_float_frame(width, height);
    Quad f3 = Quad::make_float_frame(width, height);
    Quad *quads[] = {&f1, &f2, &f3};
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        if (left_click.released) {
            for (int i = 0; i < 2; i++) {
                draw_frame.bind(draw_program);
                std::cout << left_click.y << std::endl;
                /* draw_frame.set_float_uniforms({
                    {"r", 1.0*(k%3 == 0)}, {"g", 1.0*(k%3 == 1)}, {"b", 1.0*(k%3 == 2)},
                    {"sigma", 0.01}, {"x0", left_click.x}, {"y0", left_click.y}, {"a", 0.2}
                });*/
                draw_frame.set_float_uniform("r", 1.0*(k%3 == 0));
                draw_frame.set_float_uniform("g", 1.0*(k%3 == 1));
                draw_frame.set_float_uniform("b", 1.0*(k%3 == 2));
                draw_frame.set_float_uniform("sigma", 0.01);
                draw_frame.set_float_uniform("x0", left_click.x);
                draw_frame.set_float_uniform("y0", left_click.y);
                draw_frame.set_float_uniform("a", 0.2);
                draw_frame.set_int_uniform("tex", quads[i]->get_value());
                draw_frame.draw();
                unbind();
                quads[i]->bind(view_program);
                quads[i]->set_int_uniform("tex", draw_frame.get_value());
                quads[i]->draw();
                unbind();
            }
        }
        quads[2]->bind(step_program);
        quads[2]->set_float_uniform("dx", 1.0/width);
        quads[2]->set_float_uniform("dy", 1.0/height);
        // quads[2]->set_float_uniforms({{"dx", 1.0/width}, {"dy", 1.0/height}});
        quads[2]->set_int_uniform("tex1", quads[0]->get_value());
        quads[2]->set_int_uniform("tex2", quads[1]->get_value());
        // quads[2]->set_int_uniforms({{"tex1", quads[0]->get_value()}, 
        //                             {"tex2", quads[1]->get_value()}});
        quads[2]->draw();
        unbind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        view_frame.bind(view_program);
        view_frame.set_int_uniform("tex", quads[2]->get_value());
        view_frame.draw();
        unbind();
        Quad *tmp = quads[0];
        quads[0] = quads[1];
        quads[1] = quads[2];
        quads[2] = tmp;
        glfwPollEvents();
        left_click.update(window);
        std::cout << glGetError() << std::endl;
        glfwSwapBuffers(window);
    }
    glDeleteProgram(view_program);
    glDeleteProgram(draw_program);
    glDeleteProgram(step_program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}