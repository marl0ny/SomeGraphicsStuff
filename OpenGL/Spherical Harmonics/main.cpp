#include "gl_wrappers/gl_wrappers.h"
#include "math_helpers/math_functions.h"
#include "input.h"
#include <GLFW/glfw3.h>
#include <complex>
#include <vector>
#include <iostream>
#include "marching_cubes/marching_cubes.cpp"
#include "marching_cubes/marching_cubes.h"
#include "marching_cubes/laplacian_smoothing.cpp"
#include "marching_cubes/laplacian_smoothing.h"
#include "spherical_harmonics/harmonics.h"
#include "colour.h"

template<typename T>
Mesh march(const T* volume, size_t xDim, size_t yDim, size_t zDim, T isoLevel);

Mesh march(const float* volume, size_t xDim, size_t yDim, size_t zDim, float isoLevel) {
	return march<float>(volume, xDim, yDim, zDim, isoLevel);
}

const int WIDTH = 700;
const int HEIGHT = 700;
using namespace gl_wrappers;


#define PI 3.141592653589793


const size_t size = 50;
constexpr size_t size3 = size*size*size;
std::vector<float> abs_vol = std::vector<float>(size3);
std::vector<Colour> colours = std::vector<Colour>(size3);

float sigma = 0.22360679775;


float scroll_value = 1.0;
void scroll_func(GLFWwindow *window, double x, double y) {
    scroll_value += (float)y*(2.0/100.0);
}


void fill_spherical_harmonics(int n, int m) {
    #pragma omp parallel for
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            for (size_t k = 0; k < size; k++) {
                float z = ((float)i - (size/2.0))/(float)size;
                float y = ((float)j - (size/2.0))/(float)size;
                float x = ((float)k - (size/2.0))/(float)size;
                float r = sqrt(x*x + y*y + z*z);
                std::complex<double> val_d = spherical_harmonics(n, m, 
                                                                 x, y, z);
                std::complex<float> val {(float)val_d.real(), 
                                         (float)val_d.imag()};
                float s = sigma;
                float radial = exp(-0.5*r*r/(s*s));
                abs_vol[size*size*i + size*j + k] = radial*norm(val);
                complex_to_colour<float>
                    (&colours[size*size*i + size*j + k], val);
            }
        }
    }
}


void fill_surface_data(std::vector<float> &data) {
    Mesh mesh = march(&abs_vol[0], size, size, size, 0.050);
    for (size_t i = 0; i < 3*mesh.faceCount; i++) {
        int index = mesh.faces[i];
        float x = mesh.vertices[index][0];
        float y = mesh.vertices[index][1];
        float z = mesh.vertices[index][2];
        data.push_back(2.5*(x/size - 0.5));
        data.push_back(2.5*(y/size - 0.5));
        data.push_back(2.5*(z/size - 0.5));
        data.push_back(mesh.normals[index][0]);
        data.push_back(mesh.normals[index][1]);
        data.push_back(mesh.normals[index][2]);
        float r = colours[size*size*(int)z + size*(int)y + (int)x].r;
        float g = colours[size*size*(int)z + size*(int)y + (int)x].g;
        float b = colours[size*size*(int)z + size*(int)y + (int)x].b;
        data.push_back(r/255.0);
        data.push_back(g/255.0);
        data.push_back(b/255.0);
        data.push_back(mesh.vertices[index][0]);
        data.push_back(mesh.vertices[index][1]);
    }
}


void fill_surface_data(std::vector<float> &data, 
                       std::vector<int> &elem_data) {
    Mesh mesh = march(&abs_vol[0], size, size, size, 0.050);
    smooth(mesh, 4);
    for (size_t i = 0; i < mesh.vertexCount; i++) {
        float x = mesh.vertices[i][0];
        float y = mesh.vertices[i][1];
        float z = mesh.vertices[i][2];
        data.push_back(2.5*(x/size - 0.5));
        data.push_back(2.5*(y/size - 0.5));
        data.push_back(2.5*(z/size - 0.5));
        data.push_back(mesh.normals[i][0]);
        data.push_back(mesh.normals[i][1]);
        data.push_back(mesh.normals[i][2]);
        float r = colours[size*size*(int)z + size*(int)y + (int)x].r;
        float g = colours[size*size*(int)z + size*(int)y + (int)x].g;
        float b = colours[size*size*(int)z + size*(int)y + (int)x].b;
        data.push_back(r/255.0);
        data.push_back(g/255.0);
        data.push_back(b/255.0);
        data.push_back(mesh.vertices[i][0]);
        data.push_back(mesh.vertices[i][1]);
    }
    for (size_t i = 0; i < mesh.faceCount; i++) {
        int i0 = mesh.faces[3*i];
        int i1 = mesh.faces[3*i + 1];
        int i2 = mesh.faces[3*i + 2];
        elem_data.push_back(i1);
        elem_data.push_back(i0);
        elem_data.push_back(i2);
    }
}


int main() {
    axis a = init_axis(.0, 1.0/sqrt(2.0), -1.0/sqrt(2.0), 0.0);
    vec4 camera_position = init_vec4(0.0, 0.0, -1.0, 0.0);
    vec4 light_source_loc = init_vec4(10.0, 5.0, 10.0, 0.0);
    quaternion q = init_q(0.5, 0.5, 0.5, 0.5);
    int m = 0, n = 1;
    double resize = 1.0;
    fill_spherical_harmonics(n, m);
    std::vector<float> data{};
    std::vector<int> elem_data{};
    // fill_surface_data(data);
    fill_surface_data(data, elem_data);
    GLFWwindow *window = init_window(WIDTH, HEIGHT);
    init_glew();
    Mouse left(GLFW_MOUSE_BUTTON_1);
    CharacterInput up(GLFW_KEY_UP);
    CharacterInput down(GLFW_KEY_DOWN);
    CharacterInput left_key(GLFW_KEY_LEFT);
    CharacterInput right_key(GLFW_KEY_RIGHT);
    CharacterInput r_key(GLFW_KEY_R);
    CharacterInput f_key(GLFW_KEY_F);
    Input *inputs[] = {&left, &up, &down, &left_key, &right_key,
                       &r_key, &f_key};
    GLuint verts = get_shader("./shaders/vertices.vert", GL_VERTEX_SHADER);
    GLuint frags = get_shader("./shaders/fragment.frag", GL_FRAGMENT_SHADER);
    GLuint program = make_program(verts, frags);
    GLuint quad_vert = get_shader("./shaders/quad.vert", GL_VERTEX_SHADER);
    GLuint quad_frag = get_shader("./shaders/quad.frag", GL_FRAGMENT_SHADER);
    GLuint quad_program = make_program(quad_vert, quad_frag);
    Quad quad {WIDTH, HEIGHT, GL_UNSIGNED_BYTE};
    // Frame3D frame{data, WIDTH, HEIGHT, GL_UNSIGNED_BYTE};
    Frame3D frame{data, elem_data, WIDTH, HEIGHT, GL_UNSIGNED_BYTE};
    frame.bind(program);
    frame.set_attributes({{"position", 3}, {"normal", 3}, {"colour", 3},
                          {"texture", 2}});
    frame.set_float_uniform("resize", 1.0);
    frame.set_vec3_uniform("lightSource", 0.0, 1.0, 1.5);
    frame.set_vec3_uniform("cameraLoc", (float *)&camera_position.elem[0]);
    frame.set_vec4_uniform("quaternionAxis", (float *)&q.elem[0]);
    frame.draw();
    unbind();

    glfwSetScrollCallback(window, scroll_func);

    glEnable(GL_DEPTH_TEST);
    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        resize = scroll_value;
        if (r_key.released || f_key.released) {
            if (r_key.released) sigma += 0.05;
            else if (f_key.released) sigma -= 0.05;
            fill_spherical_harmonics(n, m);
            std::vector<float> data{};
            // fill_surface_data(data);
            // frame.bind_array(data);
            std::vector<int> elem_data{};
            fill_surface_data(data, elem_data);
            frame.bind_arrays(data, elem_data);
        } else if (up.released || down.released || 
                   left_key.released || right_key.released) {
            if (up.released) {
                n = (n + 1) % 7;
                m = (m >= 0)? std::min(m, n): std::max(m, n);
            } 
            if (down.released) {
                n = (n == 0)? 6: n - 1;
                m = (m >= 0)? std::min(m, n): std::max(m, n); 
            }
            if (left_key.released) {
                m = (m > -n)? m - 1: n; 
            }
            if (right_key.released) {
                m = (m < n)? m + 1: -n;
            }
            fill_spherical_harmonics(n, m);
            std::vector<float> data{};
            // fill_surface_data(data);
            // frame.bind_array(data);
            std::vector<int> elem_data{};
            fill_surface_data(data, elem_data);
            frame.bind_arrays(data, elem_data);
        }
        glEnable(GL_CULL_FACE);
        frame.bind(program);
        frame.set_float_uniform("resize", resize);
        frame.set_int_uniform("isTexture", 0);
        frame.set_int_uniform("tex", 0);
        frame.set_vec3_uniform("lightSource", (float *)&light_source_loc.elem[0]);
        frame.set_vec3_uniform("cameraLoc", (float *)&camera_position.elem[0]);
        frame.set_vec4_uniform("quaternionAxis", (float *)&q.elem[0]);
        frame.draw();
        unbind();
        glDisable(GL_CULL_FACE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        quad.bind(quad_program);
        quad.set_int_uniform("tex", frame.get_value());
        quad.draw();
        unbind();
        glfwPollEvents();
        for (auto &i: inputs) i->update(window);
        if (left.pressed && left.dx != 0.0 && left.dy != 0.0) {
            vec4 mouse_delta = init_vec4(left.dx, left.dy, 0.0, 0.0);
            vec4 tmp0 = mul_q(&mouse_delta, &camera_position);
            copy_inplace_q(&a, &tmp0);
            normalize_inplace4(&a);
            a.angle = 7.0*left.get_delta();
            quaternion tmp1 = rotation_axis_q(&a);
            quaternion tmp2 = mul_q(&q, &tmp1);
            copy_inplace_q(&q, &tmp2);
        }
        glfwSwapBuffers(window);
    }
    return 0;
}
