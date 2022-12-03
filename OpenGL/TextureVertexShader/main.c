#include <GLES3/gl3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gl_wrappers/gl_wrappers.h"


int main() {
    int width = 512, height = 512;
    GLFWwindow *window = init_window(width, height);
    GLuint copy_program = make_quad_program("./shaders/copy.frag");
    GLuint scale_program = make_quad_program("./shaders/scale.frag");
    GLuint zero_program = make_program("./shaders/vert.vert",
                                       "./shaders/zero.frag");
    GLuint red_program = make_quad_program("./shaders/red.frag");
    GLuint red2_program = make_program("./shaders/vert.vert",
                                       "./shaders/red.frag");
    GLuint stuff_program = make_quad_program("./shaders/stuff.frag");
    float vertices[12] = {0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0,
                          1.0, 1.0, 0.0,
                          1.0, 0.0, 0.0};
    int elements[6] = {0, 1, 2, 0, 2, 3};
    struct TextureParams texture_params = {
        .type=GL_FLOAT,
        .width=width, .height=height,
        .generate_mipmap=1,
        .wrap_s=GL_REPEAT, .wrap_t=GL_CLAMP_TO_EDGE,
        .min_filter=GL_LINEAR, .mag_filter=GL_LINEAR,
    };
    struct VertexParam vertex_params[5] = {
        {.name="position",
         .size=3,
         .type=GL_FLOAT,
         .normalized=GL_FALSE,
         .offset=0,
        },
        {.name="colour",
         .size=2,
         .type=GL_FLOAT,
         .normalized=GL_FALSE,
         .offset=3*sizeof(float),
        },
    };
    frame_id main_view = new_quad(&texture_params);
    frame_id other_view = new_general_2d_frame(&texture_params,
                                               vertices, sizeof(vertices),
                                               elements, sizeof(elements));
    frame_id quad1 = new_quad(&texture_params);
    float t = 0.0;
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        t += 0.01;
        // bind_general_2d_frame(other_view, scale_program);
        bind_quad(quad1, stuff_program);
        set_float_uniform("t", t);
        draw_unbind_quad();
        bind_general_2d_frame(other_view, red2_program);
        set_vertex_attributes(vertex_params, 2);
        set_float_uniform("scale", sin(t));
        set_sampler2D_uniform("tex", quad1);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        unbind();
        bind_quad(main_view, copy_program);
        set_sampler2D_uniform("tex", other_view);
        draw_unbind_quad();
        bind_general_2d_frame(other_view, zero_program);
        set_vertex_attributes(vertex_params, 2);
        set_float_uniform("scale", sin(t));
        set_sampler2D_uniform("tex", quad1);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        unbind();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
