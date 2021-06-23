#include "shapes/vertex_array.h"
#include "math_helpers/math_functions.h"
#include "gl_wrappers/gl_wrappers.h"
#include "image_io/bitmap.h"
#include "image_io/rgb8_file.h"
#include "input.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <functional>

using namespace gl_wrappers;

int main() {
    int width = 640, height = 640;
    GLFWwindow *window = init_window(width, height);
    init_glew();

    Mouse left(GLFW_MOUSE_BUTTON_1);
    Mouse right(GLFW_MOUSE_BUTTON_2);
    CharacterInput key_q(GLFW_KEY_Q);
    CharacterInput key_w(GLFW_KEY_W);
    CharacterInput key_up(GLFW_KEY_UP);
    CharacterInput key_down(GLFW_KEY_DOWN);
    CharacterInput key_e(GLFW_KEY_E);
    CharacterInput key_a(GLFW_KEY_A);
    axis a = init_axis(1.0, 1.0, 0.0, 0.0);
    int image_w, image_h;
    vec4 camera_position = init_vec4(0.0, 0.0, -1.0, 0.0);
    vec4 light_source_loc = init_vec4(3.0, 3.0, 0.0, 1.0);
    quaternion q = init_q(0.0, 0.0, 0.0, 1.0);

    GLuint vert_shader = get_shader("./shaders/vertices.vert",
                                    GL_VERTEX_SHADER);
    GLuint frag_shader = get_shader("./shaders/fragment.frag",
                                    GL_FRAGMENT_SHADER);
    GLuint quad_vert_shader = get_shader("./shaders/quad.vert",
                                         GL_VERTEX_SHADER);
    GLuint quad_frag_shader = get_shader("./shaders/quad.frag",
                                         GL_FRAGMENT_SHADER);
    GLuint draw_gaussian_shader = get_shader("./shaders/draw-gaussian.frag",
                                             GL_FRAGMENT_SHADER);
    GLuint cubemap_vert_shader = get_shader("./shaders/cubemap.vert",
                                            GL_VERTEX_SHADER);
    GLuint cubemap_frag_shader = get_shader("./shaders/cubemap.frag",
                                            GL_FRAGMENT_SHADER);
    GLuint draw_program = make_program(quad_vert_shader, draw_gaussian_shader);
    GLuint view_program = make_program(quad_vert_shader, quad_frag_shader);
    GLuint cubemap_program = make_program(cubemap_vert_shader,
                                          cubemap_frag_shader);
    GLuint program = make_program(vert_shader, frag_shader);

    Quad q0(width, height, GL_UNSIGNED_BYTE);
    Quad q1(width, height, GL_FLOAT);
    q1.bind(draw_program);
    q1.set_int_uniform("tex", Quad::get_blank());
    q1.set_float_uniforms({{"sigma", 0.07}, {"x0", 0.5}, {"y0", 0.5},
                            {"a", 5.0}, {"r", 1.0}});
    q1.draw();
    unbind();

    uint8_t *image = get_bitmap_contents((char *)"./textures/world.bmp",
                                         &image_w, &image_h);
    if (image == NULL) {
        fprintf(stderr, "Error\n");
    }
    ImageFrame image0 {image_w, image_h, image};
    unbind();
    free(image);

    image = get_rgb8_contents((char *)"./textures/stars.rgb8",
                              &image_w, &image_h);
    if (image == NULL) {
        fprintf(stderr, "Error\n");
    }
    std::printf("%d, %d\n", image_w, image_h);
    ImageFrame image1 {image_w, image_h, image};
    unbind();
    free(image);

    CubeMapFrame cubemap(image_w, image_h);

    std::vector<float> vertex_data {
          0.25,  0.25, -0.25, 0.25,  0.25, -0.25, 0.0, 1.0, 0.0,
          0.25, -0.25, -0.25, 0.25, -0.25, -0.25, 1.0, 0.0, 0.0,
          -0.25,  0.25, -0.25, -0.25,  0.25, -0.25, 1.0, 0.0, 0.0,
         -0.25, -0.25,  -0.25, -0.25, -0.25,  -0.25, 0.0, 1.0, 0.0,
          0.25, -0.25, 0.25, 0.25, -0.25, 0.25, 0.0, 0.0, 1.0,
          0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.0, 0.0, 1.0,
          -0.25, -0.25, 0.25, -0.25, -0.25, 0.25, 1.0, 0.0, 0.0,
          -0.25, 0.25, 0.25, -0.25, 0.25, 0.25, 0.0, 1.0, 0.0
    };
    std::vector<int> edge_data {
        0, 1, 2,
        2, 3, 1,
        1, 4, 0,
        0, 5, 4,
        4, 5, 6,
        6, 5, 7,
        7, 6, 3,
        3, 2, 7,
        0, 2, 7,
        7, 5, 0,
        1, 3, 4,
        4, 6, 3
    };
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    Frame3D frame(vertex_data, edge_data,
                width, height, GL_UNSIGNED_BYTE);
    frame.bind(program);
    // frame.set_attribute("position", 3, 6, 0);
    // frame.set_attribute("colour",3, 6, 3);
    frame.bind_arrays(vertex_data, edge_data);
    frame.set_attributes({{"position", 3}, {"normal", 3},
                          {"colour", 3}});
    frame.set_float_uniform("resize", 2.5);
    frame.draw();
    unbind();
    // frame.set_draw_type(GL_LINES);

    int sphere_n = 200000;
    std::vector<float>sphere_vertices(sphere_n, 0.0);
    vertices v {0};
    vertices_init(&v, sphere_n, &sphere_vertices[0], nullptr);
    make_sphere(&v, 1.0, 40, 40);
    for (int i = 0; i < sphere_n/12; i++) {
        float *norm = vertices_get_normal(&v, i);
        vertices_set_normal(&v, i, -norm[0], -norm[1], -norm[2]);
    }
    Frame3D frame2(sphere_vertices,
                   width, height, GL_UNSIGNED_BYTE);
    frame2.bind(program);
    frame2.set_attribute("position", 3, v.vertex_size, v.position_offset);
    frame2.set_attribute("colour", 3, v.vertex_size, v.color_offset);
    frame2.set_attribute("normal", 3, v.vertex_size, v.normal_offset);
    frame2.set_attribute("texture", 2, v.vertex_size, v.uv_offset);
    frame2.set_number_of_vertices(80000);
    frame2.set_float_uniform("resize", 1.0);
    frame2.draw();
    unbind();
    // frame2.set_draw_type(GL_LINES);

    std::vector<float>vec_field{};
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            float s = -0.5 + (float)i/10.0;
            float t = -0.5 + (float)j/10.0;
            for (int k = 0; k < 3; k++) {
                // position
                vec_field.push_back(s + (float)k/(float)20.0);
                vec_field.push_back(t + (float)k/(float)20.0);
                vec_field.push_back(0.0);
                
                // colour
                vec_field.push_back(1.0);
                vec_field.push_back(0.0);
                vec_field.push_back(0.0);
                // normal
                vec_field.push_back(0.0);
                vec_field.push_back(0.0);
                vec_field.push_back(-1.0);
                // texture
                vec_field.push_back(0.0);
                vec_field.push_back(0.0);
            }
        }
    }
    Frame3D frame3(vec_field, width, height, GL_UNSIGNED_BYTE);
    frame3.bind(program);
    frame3.set_attribute("position", 3, 11, 0);
    frame3.set_attribute("colour", 3, 11, 3);
    frame3.set_attribute("normal", 3, 11, 6);
    frame3.set_attribute("texture", 2, 11, 9);
    frame3.set_number_of_vertices(200);
    frame3.set_float_uniform("resize", 1.0);
    frame3.draw();
    unbind();
    frame3.set_draw_type(GL_LINES);
    std::function<void()> draw_frame3 = [&] {
        frame3.bind(program);
        frame3.set_vec4_uniform("quaternionAxis", (float *)&q.elem[0]);
        frame3.set_vec3_uniform("lightSource", (float *)&light_source_loc);
        frame3.set_vec3_uniform("cameraLoc", (float *)&camera_position);
        frame3.set_float_uniform("resize", 2.0);
        frame3.draw();
        unbind();
        q0.bind(view_program);
        q0.set_int_uniforms({{"tex", frame3.get_value()},
                             {"width", width},
                             {"height", height}});
        // q0.set_int_uniform("tex", frame3.get_value());
        q0.draw();
        unbind();
    };

    std::function<void()>draw_q1 = [&] {
        q0.bind(view_program);
        q0.set_int_uniform("tex", q1.get_value());
        q0.draw();
        unbind();
    };
    float resize_val = 1.0;

    // Quad q2{q0};
    // Quad q3 = q0;
    // Quad q4{Quad::make_float_frame(width, height)};

    std::function<void()>draw_frame = [&] {
        frame.bind(program);
        // frame.bind_arrays(vertex_data, edge_data);
        // frame.set_attributes({{"position", 3}, {"colour", 3}});
        frame.set_vec4_uniform("quaternionAxis", (float *)&q.elem[0]);
        frame.set_vec3_uniform("lightSource", (float *)&light_source_loc);
        frame.set_vec3_uniform("cameraLoc", (float *)&camera_position);
        frame.set_int_uniform("isTexture", 0);
        frame.set_float_uniform("resize", 2.5*resize_val);
        frame.draw();
        unbind();
        q0.bind(view_program);
        q0.set_int_uniforms({{"tex", frame.get_value()},
                             {"width", width},
                             {"height", height}});
        q0.draw();
        unbind();
    };

    std::function<void()>draw_cubemap = [&] {
        image1.draw();
        cubemap.bind(cubemap_program);
        cubemap.set_int_uniform("tex", image1.get_value());
        cubemap.set_vec4_uniform("quaternionAxis", (float *)&q.elem[0]);
        cubemap.set_float_uniforms({{"resize", 1.0}});
        cubemap.draw();
        unbind();
        q0.bind(view_program);
        q0.set_int_uniforms({{"tex", cubemap.get_value()},
                             {"width", width},
                             {"height", height}});
        q0.draw();
        unbind();
    };

    std::function<void()>draw_frame2 = [&] {
        frame2.bind(program);
        // frame.bind_arrays(vertex_data, edge_data);
        // frame.set_attributes({{"position", 3}, {"colour", 3}});
        frame2.set_vec4_uniform("quaternionAxis", (float *)&q.elem[0]);
        frame2.set_vec3_uniform("lightSource", (float *)&light_source_loc);
        frame2.set_vec3_uniform("cameraLoc", (float *)&camera_position);
        frame2.set_float_uniform("resize", resize_val);
        image0.draw();
        frame2.set_int_uniforms({{"isTexture", 1},
                                 {"tex", image0.get_value()}});
        frame2.draw();
        unbind();
        q0.bind(view_program);
        q0.set_int_uniform("tex", frame2.get_value());
        q0.draw();
        unbind();
    };

    std::vector<Input *>inputs {&left, &right,
                                &key_q, &key_w, &key_e, &key_a,
                                &key_up, &key_down};
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (key_q.pressed) draw_frame2();
        else if (key_w.pressed) draw_q1();
        else if (key_e.pressed) draw_cubemap();
        else if (key_a.pressed) draw_frame3();
        else draw_frame();
        if (key_up.pressed) resize_val *= 1.01;
        if (key_down.pressed) resize_val *= 0.99;
        scale_inplace4(&light_source_loc, resize_val);
        glfwPollEvents();
        for (auto &i: inputs) {
            i->update(window);
        }
        mat4x4 matrix = y_rotation4x4(0.03);
        vec4 tmp = matmul4(&matrix, &light_source_loc);
        copy_inplace4(&light_source_loc, &tmp);
        if (left.pressed && left.dx != 0.0 && left.dy != 0.0) {
            vec4 mouse_delta = init_vec4(left.dx, left.dy, 0.0, 0.0);
            vec4 tmp0 = mul_q(&mouse_delta, &camera_position);
            copy_inplace_q(&a, &tmp0);
            normalize_inplace4(&a);
            a.angle = 5.0*left.get_delta();
            quaternion tmp1 = rotation_axis_q(&a);
            quaternion tmp2 = mul_q(&q, &tmp1);
            copy_inplace_q(&q, &tmp2);
        }
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
