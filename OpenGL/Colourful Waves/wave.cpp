#include "gl_wrappers.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

/*std::vector<std::ifstream> open_shaders(std::string strings...) {
    std::vector<std::ifstream> streams {};
    for (auto &string: strings) {
        streams.push_back(std::ifstream (string));
    };
}*/


/*
* [0, 1, 2] -> [1, 2, 0]
*/
template <typename T>
void swap3(T *elems) {
    T tmp = elems[0];
    elems[0] = elems[1];
    elems[1] = elems[2];
    elems[2] = tmp;
}


struct mouse {
    double x, y;
    int pressed = 0;
    int released = 0;
    int w, h;
    void update(GLFWwindow *window) {
        glfwGetFramebufferSize(window, &w, &h);
        glfwGetCursorPos(window, &x, &y);
        x = x/(double)w;
        y = 1.0 - y/(double)h;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
            pressed = 1;
        } else {
            if (pressed) {
                released = 1;
            }
            pressed = 0;
        }
    }
}left_click;


int main(int argc, char **argv) {
    std::ifstream vert_stream {std::string("./shaders/vertices.vert")};
    std::ifstream view_stream {std::string("./shaders/view.frag")};
    std::ifstream make_stream {std::string("./shaders/make-wave.frag")};
    std::ifstream step_stream {std::string("./shaders/wave-step.frag")};
    if (!(vert_stream && view_stream && make_stream && step_stream)) {
        std::perror("Unable to open files");
        vert_stream.close();
        view_stream.close();
        make_stream.close();
        step_stream.close();
        std::exit(1);
    }
    std::string vert_source(10000, '\0');
    std::string view_source(10000, '\0');
    std::string make_source(10000, '\0');
    std::string step_source(10000, '\0');
    vert_stream.readsome(&vert_source[0], 10000);
    view_stream.readsome(&view_source[0], 10000);
    make_stream.readsome(&make_source[0], 10000);
    step_stream.readsome(&step_source[0], 10000);
    vert_stream.close();
    view_stream.close();
    make_stream.close();
    step_stream.close();
    int width = 720, height = 720;
    GLFWwindow *window = init_window(width, height);
    init_glew();
    GLuint vert_shader = make_vertex_shader(vert_source.c_str());
    GLuint view_shader = make_fragment_shader(view_source.c_str());
    GLuint make_shader = make_fragment_shader(make_source.c_str());
    GLuint step_shader = make_fragment_shader(step_source.c_str());
    GLuint view_program = make_program(vert_shader, view_shader);
    GLuint init_program = make_program(vert_shader, make_shader);
    GLuint step_program = make_program(vert_shader, step_shader);
    glDeleteShader(vert_shader);
    glDeleteShader(view_shader);
    glDeleteShader(make_shader);
    glDeleteShader(step_shader);
    Quad f0 = Quad::make_frame(width, height);
    Quad f1 = Quad::make_float_frame(width, height);
    Quad f2 = Quad::make_float_frame(width, height);
    Quad f3 = Quad::make_float_frame(width, height);
    Quad f4 = Quad::make_float_frame(width, height);
    Quad *quads[] = {&f2, &f3, &f4};
    for (int i = 0; !glfwWindowShouldClose(window); i++) {
        if (left_click.released) {
            for (int i = 0; i < 2; i++) {
                f1.set_program(init_program);
                f1.bind();
                f1.set_float_uniforms(std::map<std::string, double> {
                    {"angle", 0.0}, {"scale", 1.0},
                    {"r", 1.0 + (double)(random()%100)/200.0}, 
                    {"g", 1.0 + (double)(random()%100)/200.0}, 
                    {"b", 1.0 + (double)(random()%100)/200.0},
                    {"sigma", 0.01}, {"x0", left_click.x}, {"y0", left_click.y}, {"a", 0.2}
                });
                f1.set_int_uniform("tex", quads[i]->get_value());
                f1.draw();
                unbind();
                quads[i]->set_program(view_program);
                quads[i]->bind();
                quads[i]->set_float_uniform("scale", 1.0);
                quads[i]->set_int_uniform("tex", f1.get_value());
                quads[i]->draw();
                unbind();
            }
            left_click.released = false;
        }
        quads[2]->set_program(step_program);
        quads[2]->bind();
        quads[2]->set_float_uniform("scale", 1.0);
        quads[2]->set_float_uniform("dx", 1.0/width);
        quads[2]->set_float_uniform("dy", 1.0/height);
        quads[2]->set_int_uniform("tex1", quads[0]->get_value());
        quads[2]->set_int_uniform("tex2", quads[1]->get_value());
        quads[2]->draw();
        unbind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        f0.set_program(view_program);
        f0.bind();
        f0.set_int_uniforms(std::map<std::string, int> {
            {"tex", quads[2]->get_value()}
        });
        f0.set_float_uniforms(std::map<std::string, double> 
            {{"angle", 0.0}, {"scale", 1.0}});
        f0.draw();
        unbind();
        swap3<Quad *>(quads);
        glfwPollEvents();
        left_click.update(window);
        glfwSwapBuffers(window);
    }
    glDeleteProgram(view_program);
    glDeleteProgram(init_program);
    glDeleteProgram(step_program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}