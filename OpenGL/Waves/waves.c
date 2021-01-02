#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "read_file.h"
#include "gl_wrappers.h"

# define type__ float

int main(int argc, char **argv) {
    int screen_w = 512;
    int screen_h = 512;
    GLFWwindow *window = init_window(screen_w, screen_h);
    char *vs_source = get_file_contents("./shaders/vertex_shader.vert");
    if (vs_source == NULL) {
        fprintf(stderr, "Unable to open file.");
        exit(1);
    }
    char *fs_source = get_file_contents("./shaders/fragment_shader.frag");
    if (fs_source == NULL) {
        fprintf(stderr, "Unable to open file.");
        exit(1);
    }
    init_glew();
    type__ *image = malloc(2000*2000*sizeof(type__));
    int w = 2*screen_w;
    int h = 2*screen_h;
    float data[] = {1.0, 1.0, 0.0,
                    1.0, -1.0, 0.0,
                    -1.0, -1.0, 0.0,
                    -1.0, 1.0, 0.0};
    int elements[] = {0, 1, 2, 2, 3, 0};
    GLuint vs = make_vertex_shader((const char *)vs_source);
    GLuint fs = make_fragment_shader((const char *)fs_source);
    GLuint program = make_program(vs, fs);
    GLuint vao = make_vertex_array_object();
    GLuint vbo = make_vertex_buffer_object(data, sizeof(data));
    init_attrib(program, "position", 3, 3, 0);
    GLint tex = glGetUniformLocation(program, "tex1");
    GLint tex2 = glGetUniformLocation(program, "tex2");
    GLint angle = glGetUniformLocation(program, "angle");
    GLint scale = glGetUniformLocation(program, "scale");
    glUniform1f(scale, 1.0);
    glUniform1i(tex2, 3);
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    int number_of_frames = 4;
    struct FrameTextureRender frames[number_of_frames];
    GLint view_frame = 0;
    GLint frame_tex1 = 0, frame_tex2 = 0, input_tex = 0, frame_angle = 0, frame_scale = 0;
    for (int i = 0; i < number_of_frames; i++) {
        frames[i] = make_frame(screen_w, screen_h);
        // glBindVertexArray(0);
        frames[i].vao = make_vertex_array_object();
        frames[i].vbo = make_vertex_buffer_object(data, sizeof(data));
        glGenBuffers(1, &frames[i].ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frames[i].ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
        init_attrib(program, "position", 3, 3, 0);
    }
    double mouse[2] = {0.0};
    int mouse_pressed = 0;
    int mouse_released = 0;
    GLuint texture;
    GLuint tex_units[3] = {0, 1, 2};

    for (int i = 0; !glfwWindowShouldClose(window); i++) {

        if (mouse_released) {
            double amp = 1.0;
            for (int i = 1; i < w-1; i++) {
                for (int j = 1; j < h-1; j++) {
                    int x_val = i - (int)mouse[0];
                    int y_val = j - (int)mouse[1];
                    double xd = (double)x_val, yd = (double)y_val;
                    double val = (amp*exp((-xd*xd - yd*yd)/
                                  ((sqrt(2.0)*(double)w/50.0)*
                                  (sqrt(2.0)*(double)w/50.0))));
                    image[2*(h-j)*w + 2*i] = (type__)val;
                }
            }
            texture = make_texture(image, w, h);
            glUniform1i(tex, 0);    
            for (int k = 0; k < 4; k++) {
                // glActiveTexture(GL_TEXTURE0);
                if (k < 3) {
                    glBindFramebuffer(GL_FRAMEBUFFER, frames[tex_units[k]].fbo);
                    glBindVertexArray(frames[tex_units[k]].vao);
                } else {
                    glBindFramebuffer(GL_FRAMEBUFFER, frames[3].fbo);
                    glBindVertexArray(frames[3].vao);
                }
                view_frame = glGetUniformLocation(program, "isViewFrame");
                glBindTexture(GL_TEXTURE_2D, texture);
                frame_tex1 = glGetUniformLocation(program, "tex1");
                frame_tex2 = glGetUniformLocation(program, "tex2");
                input_tex = glGetUniformLocation(program, "inputTex");
                frame_scale = glGetUniformLocation(program, "scale");
                glUniform1i(view_frame, 1);
                glUniform1i(frame_tex1, 0);
                glUniform1i(frame_tex2, 0);
                glUniform1i(input_tex, tex_units[k] + 1);
                glUniform1f(frame_scale, 1.0);
                glUniform1f(angle, 0.0);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
            mouse_released = 0;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, frames[tex_units[2]].fbo);
        // glClearColor(0.5, 0.5, 0.5, 1.0);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(frames[tex_units[2]].vao);
        view_frame = glGetUniformLocation(program, "isViewFrame");
        frame_tex1 = glGetUniformLocation(program, "tex1");
        frame_tex2 = glGetUniformLocation(program, "tex2");
        input_tex = glGetUniformLocation(program, "inputTex");
        frame_scale = glGetUniformLocation(program, "scale");
        glUniform1i(view_frame, 0);
        glUniform1i(frame_tex1, tex_units[0] + 1);
        glUniform1i(frame_tex2,  tex_units[1] + 1);
        glUniform1i(input_tex, 0);
        glUniform1f(frame_scale, 1.0);
        glUniform1f(angle, 0.0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glActiveTexture(GL_TEXTURE0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(vao);
        frame_tex1 = glGetUniformLocation(program, "tex1");
        frame_tex2 = glGetUniformLocation(program, "tex2");
        view_frame = glGetUniformLocation(program, "isViewFrame");
        glUniform1i(view_frame, 1);
        glUniform1i(frame_tex1, tex_units[2] + 1);
        glUniform1i(frame_tex2, tex_units[2] + 1);
        input_tex = glGetUniformLocation(program, "inputTex");
        glUniform1i(input_tex, 4);
        glUniform1f(scale, 1.0);
        glUniform1f(angle, 0.0);
        glActiveTexture(tex_units[2] + GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, frames[tex_units[2]].texture);
        glActiveTexture(GL_TEXTURE0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        GLuint tmp = tex_units[0];
        tex_units[0] = tex_units[1];
        tex_units[1] = tex_units[2];
        tex_units[2] = tmp;

        glfwPollEvents();

        glfwGetCursorPos(window, &mouse[0], &mouse[1]);
        glfwGetFramebufferSize(window, &w, &h);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
            mouse_pressed = 1;
        } else {
            if (mouse_pressed) {
                mouse_released = 1;
            }
            mouse_pressed = 0;
        }
        glfwSwapBuffers(window);
    }
    return 0;
}
