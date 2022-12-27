#include <GLES3/gl3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "gl_wrappers.h"
#include "gl_wrappers/gl_wrappers.h"

#ifdef __APPLE__
const int VIEW_WIDTH = 1024;
const int VIEW_HEIGHT = 1024;
#else
const int VIEW_WIDTH = 512;
const int VIEW_HEIGHT = 512;
#endif
const int N_PARTICLES = 16384;

struct Vec4 init_pos_vel[N_PARTICLES];
struct Vec4 init_masses[N_PARTICLES];


void reduce_texture2D_to_texture1D(int scale_program, frame_id init_quad,
                                   frame_id *sum_quads, size_t init_size) {
    int i = 0;
    for (int size=init_size/2; size >= 1; i++, size /= 2) {
        glViewport(0, 0, size, init_size);
        bind_quad(sum_quads[i], scale_program);
        set_float_uniform("scale", 2.0);
        frame_id tmp = (i == 0)? init_quad: sum_quads[i-1];
        set_sampler2D_uniform("tex", tmp);
        draw_unbind_quad();
    }
    glViewport(0, 0, init_size, init_size);
}

struct TimestepArgs {
    GLuint timestep_program;
    GLuint force_program, force_for_program, scale_program;
    frame_id *sum_quads, *pos_vel;
    frame_id masses, force_each, force_for_each;
    size_t len_sum_quads, init_size;
    float grav_const, dt;
};

void timestep(struct TimestepArgs *args) {
    GLuint timestep_program = args->timestep_program;
    GLuint force_program = args->force_program;
    GLuint force_for_program = args->force_for_program;
    GLuint scale_program = args->scale_program;
    frame_id *sum_quads = args->sum_quads;
    size_t len_sum_quads = args->len_sum_quads;
    size_t init_size = args->init_size;
    frame_id *pos_vel = args->pos_vel;
    frame_id masses = args->masses, force_each = args->force_each;
    frame_id force_for_each = args->force_for_each;
    float grav_const = args->grav_const, dt = args->dt;
    if (force_for_each <= 0) {
        glViewport(0, 0, init_size, init_size);
        bind_quad(force_each, force_program);
        set_sampler2D_uniform("massTex", masses);
        set_sampler2D_uniform("posVelTex", pos_vel[1]);
        set_float_uniform("G", grav_const);
        draw_unbind_quad();
        reduce_texture2D_to_texture1D(scale_program, force_each,
                                      sum_quads, init_size);
    } else {
        glViewport(0, 0, 1, init_size);
        bind_quad(force_for_each, force_for_program);
        set_sampler2D_uniform("massTex", masses);
        set_sampler2D_uniform("posVelTex", pos_vel[1]);
        set_float_uniform("G", grav_const);
        set_float_uniform("nParticles", (float)init_size);
        draw_unbind_quad();
    }
    glViewport(0, 0, 1, init_size);
    bind_quad(pos_vel[2], timestep_program);
    set_sampler2D_uniform("massTex", masses);
    set_sampler2D_uniform("posVelTex0", pos_vel[0]);
    set_sampler2D_uniform("posVelTex1", pos_vel[1]);
    if (force_for_each <= 0) {
        set_sampler2D_uniform("forceTex", sum_quads[len_sum_quads-1]);
    } else {
        set_sampler2D_uniform("forceTex", force_for_each);
    }
    set_float_uniform("dt", dt);
    set_int_uniform("method", 0);
    draw_unbind_quad();
}

void roll3(frame_id *arr) {
    frame_id tmp[3];
    tmp[0] = arr[0], tmp[1] = arr[1], tmp[2] = arr[2];
    arr[0] = tmp[1], arr[1] = tmp[2], arr[2] = tmp[0];
}

int main() {

    // Initialize the window
    GLFWwindow *window = init_window(VIEW_WIDTH, VIEW_HEIGHT);

    // Make the shader programs
    frame_id zero_program = make_quad_program("./shaders/zero.frag");
    frame_id copy_program = make_quad_program("./shaders/copy.frag");
    frame_id scale_program = make_quad_program("./shaders/scale.frag");
    frame_id force_program = make_quad_program("./shaders/force.frag");
    frame_id force_for_program = make_quad_program("./shaders/force-for.frag");
    frame_id timestep_program = make_quad_program("./shaders/timestep.frag");
    frame_id particles_view_program
        = make_program("./shaders/particle-vert.vert",
                       "./shaders/colour.frag");


    struct TextureParams texture_params = {
        .type=GL_FLOAT,
        .width=VIEW_WIDTH, .height=VIEW_HEIGHT,
        .generate_mipmap=1,
        .wrap_s=GL_REPEAT, .wrap_t=GL_REPEAT,
        .min_filter=GL_LINEAR, .mag_filter=GL_LINEAR
    };

    // Visualization frame
    frame_id view_quad = new_quad(NULL);
    frame_id quad2 = new_quad(&texture_params);

    // Start making frames involved in simulation
    texture_params.width = N_PARTICLES;
    texture_params.height = N_PARTICLES;
    frame_id force_quad = new_quad(&texture_params);
    int sum_quads_len = (int)(log(N_PARTICLES)/log(2));
    frame_id *sum_quads = malloc(sizeof(frame_id)*sum_quads_len);
    for (int i = 0; i < sum_quads_len; i++) {
        texture_params.width /= 2;
        sum_quads[i] = new_quad(&texture_params);
    }
    texture_params.width = 1;
    frame_id mass_quad = new_quad(&texture_params);
    bind_quad(mass_quad, zero_program);
    draw_unbind_quad();
    frame_id force_for_quad = new_quad(&texture_params);
    frame_id pos_vel_quads[3];
    for (int i = 0; i < 3; i++) {
        pos_vel_quads[i] = new_quad(&texture_params);
        bind_quad(pos_vel_quads[i], zero_program);
        draw_unbind_quad();
    }

    // Make frame for actually visualizing the particles.
    int sizeof_particles_view_vertices = 2*sizeof(float)*N_PARTICLES;
    struct Vec2 *particles_view_vertices
        = malloc(sizeof_particles_view_vertices);
    struct VertexParam vertex_params[2] = {
        {.name="uvIndex", .size=2, .type=GL_FLOAT,
         .normalized=GL_FALSE, .stride=2*sizeof(float), .offset=0},
    };
    for (int i = 0; i < N_PARTICLES; i++) {
        particles_view_vertices[i].ind[0] = 0.5;
        particles_view_vertices[i].ind[1] = ((float)i + 0.5)/(float)N_PARTICLES;
    }
    int sizeof_particles_view_elements = N_PARTICLES*sizeof(int);
    int *particles_view_elements = malloc(sizeof_particles_view_elements);
    for (int i = 0; i < N_PARTICLES; i++)
        particles_view_elements[i] = i;
    texture_params.width=VIEW_WIDTH, texture_params.height=VIEW_HEIGHT;
    frame_id particles_view
        = new_general_2d_frame(&texture_params,
                               (float *)particles_view_vertices,
                               sizeof_particles_view_vertices,
                               particles_view_elements,
                               sizeof_particles_view_elements);

    // Initial conditions
    for (int i = 0; i < N_PARTICLES; i++) {
        float rx = 0.5*(float)rand()/(float)RAND_MAX + 0.25;
        float ry = 0.5*(float)rand()/(float)RAND_MAX + 0.25;
        float vx = 1000.0*((float)rand()/(float)RAND_MAX - 0.5);
        float vy = 1000.0*((float)rand()/(float)RAND_MAX - 0.5);
        if (i == 0) rx = 0.5;
        if (i == 0) ry = 0.5;
        // float vx = 0.0;
        // float vy = 0.0;
        if (i != 0) {
            vx = -1000.0*(ry-0.5)/sqrt((ry-0.5)*(ry-0.5) + (rx-0.5)*(rx-0.5));
            vy = 1000.0*(rx-0.5)/sqrt((ry-0.5)*(ry-0.5) + (rx-0.5)*(rx-0.5));
        }
        if (i == 0) vx = 0.0;
        if (i == 0) vy = 0.0;
        printf("%f\n", vx);
        // particles_view_vertices.ind[0] = rx;
        // particles_view_vertices.ind[1] = ry;
        // particles_view_vertices.ind[2] = vx;
        // particles_view_vertices.ind[3] = vy;
        init_pos_vel[i].ind[0] = rx;
        init_pos_vel[i].ind[1] = ry;
        init_pos_vel[i].ind[2] = vx;
        init_pos_vel[i].ind[3] = vy;
        init_masses[i].ind[0] = (i == 0)? 10000000.0: 1.0;
        for (int k = 1; k < 4; k++) init_masses[i].ind[k] = 0.0;
    }
    for (int i = 0; i < 3; i++) {
        quad_substitute_array(pos_vel_quads[i],
                              1, N_PARTICLES, GL_FLOAT, init_pos_vel);
    }
    quad_substitute_array(mass_quad, 1, N_PARTICLES, GL_FLOAT, init_masses);

    get_quad_texture_array(pos_vel_quads[0], 0, 0, 1,
                           N_PARTICLES, GL_FLOAT, init_pos_vel);



    for (int k = 0; !glfwWindowShouldClose(window); k++) {
        float dt = 0.00001;
        float grav_const = 1.0;
        struct TimestepArgs args = {
            .timestep_program=timestep_program,
            .force_program=force_program,
            .force_for_program=force_for_program,
            .scale_program=scale_program,
            .sum_quads=sum_quads, .pos_vel=pos_vel_quads,
            .masses=mass_quad, .force_each=force_quad,
            .force_for_each=force_for_quad,
            // .force_for_each=-1,
            .len_sum_quads=sum_quads_len, .init_size=N_PARTICLES,
            .grav_const=grav_const, .dt=(k == 0)? 0.5*dt: dt,
        };
        for (int i = 0; i < 1; i++) {
            timestep(&args);
            roll3(pos_vel_quads);
        }
        get_quad_texture_array(pos_vel_quads[1], 0, 0, 1,
                           N_PARTICLES, GL_FLOAT, init_pos_vel);
        glViewport(0, 0, VIEW_WIDTH, VIEW_HEIGHT);

        bind_general_2d_frame(particles_view,
                              particles_view_program);
        set_vertex_attributes(vertex_params, 1);
        set_sampler2D_uniform("tex", pos_vel_quads[0]);
        set_vec4_uniform("colour", 1.0, 0.75, 0.5, 1.0);
        glDrawArrays(GL_POINTS, 0, N_PARTICLES);
        unbind();

        bind_quad(view_quad, copy_program);
        set_sampler2D_uniform("tex", particles_view);
        draw_unbind_quad();

        swap_fbo(particles_view, quad2);
        bind_quad(quad2, zero_program);
        draw_unbind_quad();
        swap_fbo(particles_view, quad2);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    get_quad_texture_array(pos_vel_quads[0], 0, 0, 1,
                           N_PARTICLES, GL_FLOAT, init_pos_vel);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;

}
