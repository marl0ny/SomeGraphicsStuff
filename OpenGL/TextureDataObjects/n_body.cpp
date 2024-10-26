/* N-body gravity simulation.
 *
 * References:
 *
 * Verlet integration:
 *  - Wikipedia contributors. (2023, April 24).
 *  Verlet integration. In Wikipedia, The Free Encyclopedia.
 * 
 */
#include "n_body.hpp"

// #include <OpenGL/OpenGL.h>
#include <cstdlib>
#include <new>
#define GL_SILENCE_DEPRECATION

#include "texture_data.hpp"
#include "draw_texture_data.hpp"
// #include <OpenGL/OpenGL.h>
#include <vector>
#include "fft.h"
#include <iostream>
#include <ctime>
#include "bitmap.h"
#include "summation.h"
#include <GLES3/gl3.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <functional>

static std::function <void()> loop;
#ifdef __EMSCRIPTEN__
static void main_loop() {
    loop();
}
#endif

int particles_coulomb(Renderer *renderer) {
    int main_frame = renderer->main_frame;
    GLFWwindow *window = renderer->window;
    int exit_status = 0;
    int window_width {}, window_height {};
    window_dimensions(window, &window_width, &window_height);
    int N_PARTICLES = 2048;
    double dt = 0.000003;
    frame_id view_program
        = make_quad_program("./shaders/util/copy.frag");

    auto p0_arr = std::vector<struct Vec2> {};
    auto v0_arr = std::vector<struct Vec2> {};
    for (int i = 0; i < N_PARTICLES; i++) {
        float rx = 0.7F*((float)rand()/(float)RAND_MAX - 0.5F);
        float ry = 0.7F*((float)rand()/(float)RAND_MAX - 0.5F);
        p0_arr.push_back({.x = 0.5F + rx, .y = 0.5F + ry}
            );
        v0_arr.push_back({.x = -800.0F*ry, .y = 800.0F*rx});
    }

    auto r = Texture2DData((struct Vec2 *)&p0_arr[0], N_PARTICLES, 1);
    auto v = Texture2DData((struct Vec2 *)&v0_arr[0], N_PARTICLES, 1);


    auto force_com
        = DrawTexture2DData(Path("./shaders/particles/force-coulomb.frag"));
    auto energy_int_com
        = DrawTexture2DData(
            Path("./shaders/particles/energy-int-coulomb.frag"));
    Texture2DData force_pairs0 = funcs2D::zeroes(FLOAT2, 
        N_PARTICLES, N_PARTICLES);
    Texture2DData force_pairs1 = funcs2D::zeroes(FLOAT2, 
        N_PARTICLES, N_PARTICLES);
    auto step = [&](int step_number,
                    const Texture2DData &r0, const Texture2DData &v0
                    ) -> std::vector<Texture2DData> {
        Texture2DData energy_pairs = funcs2D::zeroes(FLOAT, 
            N_PARTICLES, N_PARTICLES);
        // Texture2DData int_energy_pairs
        //   = zeroes(FLOAT, N_PARTICLES, N_PARTICLES);
        glViewport(0, 0, N_PARTICLES, N_PARTICLES);
        if (step_number == 0)
            force_com.draw(force_pairs0, "positionsTex", r0);
        Texture2DData force0 = force_pairs0.reduce_to_row();
        glViewport(0, 0, N_PARTICLES, 1);
        Texture2DData r1 = r0 + v0*dt + (0.5*dt*dt)*force0;
        glViewport(0, 0, N_PARTICLES, N_PARTICLES);
        force_com.draw(force_pairs1, "positionsTex", r1);
        Texture2DData force1 = force_pairs1.reduce_to_row();
        glViewport(0, 0, N_PARTICLES, 1);
        Texture2DData v1 = v0 + (0.5*dt)*(force0 + force1);
        double ke = 0.5*v1.squared_norm().as_double;
        glViewport(0, 0, N_PARTICLES, N_PARTICLES);
        energy_int_com.draw(energy_pairs, "positionsTex", r1);
        double pe = energy_pairs.sum_reduction().as_double;
        std::cout << ke + pe << std::endl;
        return std::vector<Texture2DData> {r1, v1};
    };

    // Particles view
    int particles_program = make_program(
        "./shaders/particles/particles.vert",
        "./shaders/util/colour.frag");
    std::string uvIndexStr ("uvIndex");
    struct VertexParam vertex_params[1] = {{
        .name=&uvIndexStr[0], .size=2, .type=GL_FLOAT,
        .normalized=GL_FALSE, .stride=2*sizeof(float),
        .offset=0,
    },};
    int particles_frame;
    auto vertices_vec
        = std::vector<struct Vec2>((size_t)N_PARTICLES);
    for (int i = 0; i < N_PARTICLES; i++) {
        vertices_vec[i].x = ((float)i + 0.5)/(float)N_PARTICLES;
        vertices_vec[i].y = 0.5;
    }
    {
        struct TextureParams tex_params = {
            .format=GL_RGBA32F,
            .width=window_width, .height=window_height,
            .generate_mipmap=1, .wrap_s=GL_CLAMP_TO_EDGE,
            .wrap_t=GL_CLAMP_TO_EDGE, .min_filter=GL_LINEAR,
            .mag_filter=GL_LINEAR,
        };
        particles_frame = new_frame(&tex_params, (float *)&vertices_vec[0],
                                    vertices_vec.size()*sizeof(struct Vec2),
                                    NULL, -1);
    }

    int k = 0;
    bool exit_loop = false;
    loop = [&] {
        int steps_per_frame = 10;
        for (int i = 0; i < steps_per_frame; i++) {
            std::vector<Texture2DData> tmp
                = step(k*steps_per_frame + i, r, v);
            swap(force_pairs0, force_pairs1);
            r = tmp[0], v = tmp[1];
        }
        glViewport(0, 0, window_width, window_height);
        bind_frame(particles_frame, particles_program);
        set_vertex_attributes(vertex_params, 1);
        r.set_as_sampler2D_uniform("tex");
        set_vec4_uniform("colour", 1.0, 1.0, 1.0, 1.0);
        glDrawArrays(GL_POINTS, 0, N_PARTICLES);
        unbind();
        glViewport(0, 0, window_width, window_height);
        bind_quad(main_frame, view_program);
        set_sampler2D_uniform("tex", particles_frame);
        // v1.set_as_sampler2D_uniform("tex");
        draw_unbind_quad();
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && k > 30)
            exit_loop = true;
        if (glfwWindowShouldClose(window)) {
            exit_status = 1;
            exit_loop = true;
        }
        glfwSwapBuffers(window);
        k++;
    };

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
    #else
    while (!exit_loop) loop();
    #endif
    return exit_status;
}
