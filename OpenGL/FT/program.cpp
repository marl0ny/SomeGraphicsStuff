#include "fft.hpp"
#include "gl_wrappers.hpp"
#include <GLFW/glfw3.h>
#include <cmath>
#include <thread>
#include <ctime>
#include <omp.h>

struct FloatRGBA {
    union {
        struct {
            float r;
            float g;
            float b;
            float a;
        };
        struct {
            float re;
            float im;
            float _pad[2];
        };
    };
    inline double real() {
        return re;
    }
    inline double imag() {
        return im;
    }
    inline void real(double re) {
        this->re = re;
    }
    inline void imag(double im) {
        this->im = im;
    }
};


void call_inplace_fft(FloatRGBA *arr, int w, int i) {
    inplace_fft<FloatRGBA>((FloatRGBA *)arr + i*w, w);
}

template <typename T>
void button_update(GLFWwindow *window, int key,
                   T &param, T new_val) {
    if (glfwGetKey(window, key) == GLFW_PRESS) param = new_val;
}


void fft(GLuint program, Quad *quads[2], float w, bool is_vert) {
    for (float block_size = 2.0; block_size <= w; block_size *= 2.0) {
        quads[1]->set_program(program);
        quads[1]->bind();
        quads[1]->set_int_uniform("tex", quads[0]->get_value());
        quads[1]->set_int_uniform("isVertical", (int)is_vert);
        quads[1]->set_float_uniform("blockSize", block_size/w);
        quads[1]->draw();
        unbind();
        Quad *tmp = quads[1];
        quads[1] = quads[0];
        quads[0] = tmp;
    }
}

void fft_shift(GLuint program, Quad *quads[2], bool is_vert) {
    quads[1]->set_program(program);
    quads[1]->bind();
    quads[1]->set_int_uniform("tex", quads[0]->get_value());
    quads[1]->set_int_uniform("isVertical", is_vert);
    quads[1]->draw();
    unbind();
    Quad *tmp = quads[1];
    quads[1] = quads[0];
    quads[0] = tmp;
}


int main() {
    int w = 512, h = 512;
    uint8_t *image = new uint8_t[h*w*4];
    bool key_pressed = false;
    int view_mode = 0;
    double x0 = 0.5, y0 = 0.5;
    double k = 50, j = 50;
    double sigma = 0.025;
    for (int i = 0; i < w*h; ++i) {
        image[i*4] = (uint8_t)i;
        image[i*4 + 1] = (uint8_t)i;
        image[i*4 + 2] = (uint8_t)i;
        image[i*4 + 3] = (uint8_t)1;
    }
    for (int i = 0; i < h; i++) {
        bitreverse2<int>((int *)&image[w*i*4], w);
    }
    GLFWwindow *window = init_window(3*w, h);
    glViewport(0, 0, w, h);
    init_glew();
    GLuint vert_shader = get_shader("./shaders/vertices.vert", GL_VERTEX_SHADER);
    // GLuint r2_vert_shader = get_shader("./shaders/reverse2-vertices.vert", GL_VERTEX_SHADER);
    GLuint make_tex_shader = get_shader("./shaders/make-tex.frag", GL_FRAGMENT_SHADER);
    GLuint view_shader = get_shader("./shaders/view.frag", GL_FRAGMENT_SHADER);
    // GLuint sort_shader = get_shader("./shaders/reverse-bit-sort2.frag", GL_FRAGMENT_SHADER);
    GLuint hpas_shader = get_shader("./shaders/fft-iter.frag", GL_FRAGMENT_SHADER);
    GLuint fftshift_shader = get_shader("./shaders/fftshift.frag", GL_FRAGMENT_SHADER);
    GLuint make_tex_program = make_program(vert_shader, make_tex_shader);
    GLuint view_program = make_program(vert_shader, view_shader);
    // GLuint sort_program = make_program(r2_vert_shader, view_shader);
    GLuint hpas_program = make_program(vert_shader, hpas_shader);
    GLuint fftshift_program = make_program(vert_shader, fftshift_shader);

    Quad q0 = Quad::make_frame(3.0*w, h);

    GLint tex = make_texture(image, w, h);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    Quad q2 = Quad::make_float_frame(w, h);
    Quad q3 = Quad::make_float_frame(w, h);
    Quad q4 = Quad::make_float_frame(w, h);

    q2.set_program(make_tex_program);
    q2.bind();
    q2.set_int_uniform("type", 0);
    q2.set_float_uniforms({
        {"k", 50},
        {"sigma", 0.025}, {"a", 5.0},
        {"x0", 0.5}, {"y0", 0.5}, {"r", 1.0}
    });
    q2.set_int_uniform("tex", Quad::get_blank());
    q2.draw();
    FloatRGBA *arr = new FloatRGBA[w*h];
    // clock_t t1 = clock();
    double t1 = omp_get_wtime();
    q2.get_texture_array(arr, 0, 0, w, h, GL_FLOAT);
    inplace_fft2<FloatRGBA>(arr, w);
    inplace_fftshift2(arr, w);
    q3.substitute_array(w, h, GL_FLOAT, arr);
    inplace_fftshift2(arr, w);
    inplace_ifft2<FloatRGBA>(arr, w);
    // inplace_fftshift2(arr, w);
    q4.substitute_array(w, h, GL_FLOAT, arr);
    // clock_t t2 = clock();
    double t2 = omp_get_wtime();
    unbind();
    // double time_taken = (double)(t2 - t1)/(double)CLOCKS_PER_SEC;
    double time_taken = t2 - t1;
    std::cout << "Time taken: " << time_taken
                                 << "\n";

    Quad *quads[3] = {&q2, &q3, &q4};
    double brightness = 1.0;
    for (int i = 0; !glfwWindowShouldClose(window); i++) {
        if (key_pressed) {
            q2.set_program(make_tex_program);
            q2.bind();
            q2.set_int_uniform("type", view_mode);
            q2.set_float_uniforms({
                    {"k", k}, {"j", j},
                    {"sigma", sigma}, {"a", 5.0},
                    {"x0", x0}, {"y0", y0}, {"r", 1.0}
            });
            q2.set_int_uniform("tex", Quad::get_blank());
            q2.draw();
            // unbind();
            q2.get_texture_array(arr, 0, 0, w, h, GL_FLOAT);
            inplace_fft2<FloatRGBA>(arr, w);
            inplace_fftshift2(arr, w);
            q3.substitute_array(w, h, GL_FLOAT, arr);
            inplace_fftshift2(arr, w);
            inplace_ifft2<FloatRGBA>(arr, w);
            // inplace_fftshift2(arr, w);
            q4.substitute_array(w, h, GL_FLOAT, arr);
            key_pressed = false;
            unbind();
        }
        glViewport(0, 0, 3*w, h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        q0.set_program(view_program);
        q0.bind();
        q0.set_int_uniforms({
            {"tex", quads[0]->get_value()},
            {"tex2", quads[1]->get_value()},
            {"tex3", quads[2]->get_value()},
            // {"tex", q3.get_value()}
        });
        q0.set_float_uniform("gridSize", w);
        q0.set_float_uniform("brightness", brightness);
        q0.draw();
        unbind();
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            brightness *= 1.1;
        } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            brightness *= 0.9;
        }
        glfwSwapBuffers(window);
        button_update(window, GLFW_KEY_Q, sigma, sigma - 0.0025);
        button_update(window, GLFW_KEY_E, sigma, sigma + 0.0025);
        button_update(window, GLFW_KEY_W, y0, y0+0.01);
        button_update(window, GLFW_KEY_A, x0, x0-0.01);
        button_update(window, GLFW_KEY_S, y0, y0-0.01);
        button_update(window, GLFW_KEY_D, x0, x0+0.01);
        button_update(window, GLFW_KEY_R, k, k + 1.0);
        button_update(window, GLFW_KEY_F, k, k - 1.0);
        button_update(window, GLFW_KEY_T, j, j + 1.0);
        button_update(window, GLFW_KEY_G, j, j - 1.0);
        for (int i = 0; i < 7; i++) {
            button_update(window, GLFW_KEY_0+i, view_mode, i);
        }
        for (int i = 0; i < 7; i++) {
            button_update(window, GLFW_KEY_0+i, key_pressed, true);
        }
        button_update(window, GLFW_KEY_Q, key_pressed, true);
        button_update(window, GLFW_KEY_E, key_pressed, true);
        button_update(window, GLFW_KEY_W, key_pressed, true);
        button_update(window, GLFW_KEY_A, key_pressed, true);
        button_update(window, GLFW_KEY_S, key_pressed, true);
        button_update(window, GLFW_KEY_D, key_pressed, true);
        button_update(window, GLFW_KEY_R, key_pressed, true);
        button_update(window, GLFW_KEY_F, key_pressed, true);
        button_update(window, GLFW_KEY_T, key_pressed, true);
        button_update(window, GLFW_KEY_G, key_pressed, true);
        glViewport(0, 0, w, h);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}