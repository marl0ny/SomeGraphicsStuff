#include "interface.h"
#include <GLES3/gl3.h>
#include <math.h>
#include "interface.h"
#include <stdio.h>
#include <stdlib.h>
#include "gl_wrappers/gl_wrappers.h"


struct Programs {
    GLuint copy, zero, scale, colour;
    GLuint points_density;
    GLuint points_colour;
    GLuint init_gaussian;
    GLuint init_boundary;
    GLuint gradient;
    GLuint gaussian_blur;
    GLuint sample_vol;
    GLuint show_vol;
};

struct TextureDimensions {
    int width_3d, height_3d, length_3d;
    int width_2d, height_2d;
};

struct SimParams {
    int view_width, view_height;
    struct TextureDimensions texture_dimensions;
    struct TextureDimensions render_texture_dimensions;
    struct DVec4 rotation;
    struct DVec4 rotation2;
    float scale;
    struct Vec3 translate;
};

struct Frames {
    frame_id main_view;
    frame_id sub_view1, sub_view2, sub_view3;
    frame_id draw;
    frame_id gradient;
    frame_id boundary_mask;
};


static struct Programs s_programs = {};
static struct SimParams s_sim_params = {};
static struct Frames s_frames = {};

static int s_sizeof_vertices = 0;
static int s_sizeof_elements = 0;

void init_programs(struct Programs *programs) {
    programs->zero = make_quad_program("./shaders/zero.frag");
    programs->copy = make_quad_program("./shaders/copy.frag");
    programs->scale = make_quad_program("./shaders/scale.frag");
    programs->colour = make_quad_program("./shaders/colour.frag");
    programs->points_density = make_program("./shaders/points-density.vert",
                                            "./shaders/pixel.frag");
    programs->points_colour = make_program("./shaders/points-density.vert",
                                            "./shaders/colour.frag");
    programs->init_boundary
        = make_quad_program("./shaders/init-boundary.frag");
    programs->init_gaussian
        = make_quad_program("./shaders/init-gaussian.frag");
    programs->gradient
        = make_quad_program("./shaders/gradient.frag");
    programs->sample_vol = make_program("./shaders/sample-vol.vert",
                                        "./shaders/sample-vol.frag");
    programs->gaussian_blur
        = make_quad_program("./shaders/gaussian-blur.frag");
    programs->show_vol = make_program("./shaders/show-vol.vert",
                                      "./shaders/show-vol.frag");
}

void init_sim_params(struct SimParams *params) {
    #ifdef __APPLE__
    params->view_width = 1024;
    params->view_height = 1024;
    #else
    params->view_width = 512;
    params->view_height = 512;
    #endif
    params->rotation.x = 0.0;
    params->rotation.y = 0.0;
    params->rotation.z = 0.0;
    params->rotation.w = 1.0;
    params->rotation2.x = 0.0;
    params->rotation2.y = 0.0;
    params->rotation2.z = 0.0;
    params->rotation2.w = 1.0;
    params->scale = 1.0;
    params->translate.x = -0.5;
    params->translate.y = -0.5;
    params->translate.z = -0.5;
    params->texture_dimensions.width_2d = 512;
    params->texture_dimensions.height_2d = 512;
    params->texture_dimensions.width_3d = 64;
    params->texture_dimensions.height_3d = 64;
    params->texture_dimensions.length_3d = 64;
    params->render_texture_dimensions.width_2d = 1024;
    params->render_texture_dimensions.height_2d = 1024;
    params->render_texture_dimensions.width_3d = 64;
    params->render_texture_dimensions.height_3d = 64;
    params->render_texture_dimensions.length_3d = 256;
    /* params->render_texture_dimensions.width_2d = 2048;
    params->render_texture_dimensions.height_2d = 2048;
    params->render_texture_dimensions.width_3d = 128;
    params->render_texture_dimensions.height_3d = 128;
    params->render_texture_dimensions.length_3d = 256;*/
    s_sizeof_vertices = (4*sizeof(float)*
                         params->render_texture_dimensions.width_2d*
                         params->render_texture_dimensions.height_2d);
}

struct IVec2 to_2d_indices(int i, int j, int k,
                           const struct TextureDimensions *d) {
    int k_per_row = d->width_2d/d->width_3d;
    int i_offset = (k % k_per_row)*d->width_3d;
    int j_offset = (k / k_per_row)*d->height_3d;
    struct IVec2 st = {.i = i_offset + i, .j = j_offset + j};
    return st;
}

int to_1d_index(int i, int j, int k, const struct TextureDimensions *d) {
    struct IVec2 st = to_2d_indices(i, j, k, d);
    return st.i + st.j*d->width_2d;
}


static const int Z_ORIENTATION = 0;
static const int X_ORIENTATION = 1;
static const int Y_ORIENTATION = 2;
int single_face(int *elements, int k, int orientation,
                const struct TextureDimensions *d) {
    int inc = 1;
    int j = 0;
    int elem_index = 0;
    int horizontal_iter = d->height_3d - 1;
    int vertical_iter = d->width_3d - 2;
    if (orientation == Y_ORIENTATION) {
        horizontal_iter = d->length_3d - 1;
        vertical_iter = d->width_3d - 2;
    } else if (orientation == X_ORIENTATION) {
        horizontal_iter = d->width_3d - 1;
        vertical_iter = d->height_3d - 2;
    }
    for (int i = 0; j < horizontal_iter; i += inc) {
        switch(orientation) {
            case Z_ORIENTATION:
                elements[elem_index++] = to_1d_index(i, j, k, d);
                elements[elem_index++] = to_1d_index(i+1, j, k, d);
                elements[elem_index++] = to_1d_index(i+1, j+1, k, d);
                elements[elem_index++] = to_1d_index(i+1, j+1, k, d);
                elements[elem_index++] = to_1d_index(i, j, k, d);
                elements[elem_index++] = to_1d_index(i, j+1, k, d);
                break;
            case Y_ORIENTATION:
                elements[elem_index++] = to_1d_index(i, k, j, d);
                elements[elem_index++] = to_1d_index(i+1, k, j, d);
                elements[elem_index++] = to_1d_index(i+1, k, j+1, d);
                elements[elem_index++] = to_1d_index(i+1, k, j+1, d);
                elements[elem_index++] = to_1d_index(i, k, j, d);
                elements[elem_index++] = to_1d_index(i, k, j+1,  d);
                break;
            case X_ORIENTATION:
                elements[elem_index++] = to_1d_index(k, i, j, d);
                elements[elem_index++] = to_1d_index(k, i+1, j, d);
                elements[elem_index++] = to_1d_index(k, i+1, j+1, d);
                elements[elem_index++] = to_1d_index(k, i+1, j+1, d);
                elements[elem_index++] = to_1d_index(k, i, j, d);
                elements[elem_index++] = to_1d_index(k, i, j+1, d);
                break;
        }
        if (i == 0) {
            if (inc == -1) {
                inc = 0;
                j++;
            } else if (inc == 0) {
                inc = 1;
            }
        } else if (i == vertical_iter) {
            if (inc == 1) {
                inc = 0;
                j++;
            } else if (inc == 0) {
                inc = -1;
            }
        }
    }
    return elem_index;
}

int *new_elements(int *ptr_sizeof_elements,
                  const struct TextureDimensions *d) {
    *ptr_sizeof_elements = sizeof(int)*
        (6*(d->width_3d-1)*(d->height_3d-1)*d->length_3d
         + 3*d->length_3d);
    int *elements = malloc(*ptr_sizeof_elements);
    int elem_index = 0;
    if (elements == NULL) {
        perror("malloc");
        return NULL;
    }
    for (int k = d->length_3d - 1;  k >= 0; k--) {
        elem_index += single_face(elements + elem_index, k, Z_ORIENTATION, d);
        int index = elements[elem_index - 1];
        elements[elem_index++] = index;
        elements[elem_index++] = index;
        elements[elem_index++] = to_1d_index(0, 0, (k-1)%d->length_3d, d);
    }
    return elements;
}

void init_frames(struct Frames *frames, struct SimParams *params) {
    frames->main_view = new_quad(NULL);
    struct TextureParams tex_params = {
        .type=GL_FLOAT,
        .width=params->texture_dimensions.width_2d,
        .height=params->texture_dimensions.height_2d,
        .generate_mipmap=1,
        .wrap_s=GL_CLAMP_TO_EDGE, .wrap_t=GL_CLAMP_TO_EDGE,
        .min_filter=GL_LINEAR, .mag_filter=GL_LINEAR
    };
    // Initialize all the quad frames.
    frames->draw = new_quad(&tex_params);
    frames->gradient = new_quad(&tex_params);
    frames->boundary_mask = new_quad(&tex_params);
    // Construct the non quad frames.
    struct Vec4 *vertices = malloc(s_sizeof_vertices);
    if (vertices == NULL) {
        perror("malloc");
        return;
    }
    int count = 0;
    for (int i = 0; i < params->render_texture_dimensions.width_2d; i++) {
        for (int j = 0; 
             j < params->render_texture_dimensions.height_2d; j++) {
            int w = params->render_texture_dimensions.width_2d;
            int h = params->render_texture_dimensions.height_2d;
            vertices[j*w + i].x = ((float)i + 0.5)/(float)w;
            vertices[j*w + i].y = ((float)j + 0.5)/(float)h;
            vertices[j*w + i].z = 0.0;
            vertices[j*w + i].w = 1.0;
        }
    }
    int *elements = new_elements(&s_sizeof_elements,
                                 &params->render_texture_dimensions);
    if (elements == NULL) {
        fprintf(stderr, "Error");
        return;
    }
    tex_params.type = GL_HALF_FLOAT;
    tex_params.min_filter = GL_LINEAR_MIPMAP_LINEAR;
    tex_params.mag_filter = GL_LINEAR_MIPMAP_LINEAR;
    // tex_params.width = params->render_texture_dimensions.width_2d;
    // tex_params.height = params->render_texture_dimensions.height_2d;
    tex_params.width = params->view_width;
    tex_params.height = params->view_height;
    frames->sub_view1 = new_frame(&tex_params, (float *)vertices,
                                  s_sizeof_vertices,
                                  elements, s_sizeof_elements);
    frames->sub_view2 = new_frame(&tex_params, (float *)vertices,
                                  s_sizeof_vertices,
                                  elements, s_sizeof_elements);
    tex_params.min_filter = GL_LINEAR_MIPMAP_LINEAR;
    tex_params.mag_filter = GL_LINEAR_MIPMAP_LINEAR;
    tex_params.type = GL_UNSIGNED_BYTE;
    frames->sub_view3 = new_frame(&tex_params, (float *)vertices,
                                  s_sizeof_vertices,
                                  elements, s_sizeof_elements);
    tex_params.type = GL_FLOAT;
}


void init() {
    init_programs(&s_programs);
    init_sim_params(&s_sim_params);
    init_frames(&s_frames, &s_sim_params);
    glViewport(0, 0,
               s_sim_params.texture_dimensions.width_2d,
               s_sim_params.texture_dimensions.height_2d);
    bind_quad(s_frames.draw, s_programs.init_gaussian);
    set_ivec2_uniform("texelDimensions2D",
                      s_sim_params.texture_dimensions.width_2d,
                      s_sim_params.texture_dimensions.height_2d);
    set_ivec3_uniform("texelDimensions3D",
                      s_sim_params.texture_dimensions.width_3d,
                      s_sim_params.texture_dimensions.height_3d,
                      s_sim_params.texture_dimensions.length_3d);
    set_vec3_uniform("r0", 0.5, 0.5, 0.5);
    set_vec3_uniform("colour", 0.5, 0.5, 1.0);
    set_vec3_uniform("sigma", 0.05, 0.1, 0.07);
    draw_unbind_quad();
    bind_quad(s_frames.boundary_mask, s_programs.init_boundary);
    set_ivec2_uniform("texelDimensions2D",
                      s_sim_params.texture_dimensions.width_2d,
                      s_sim_params.texture_dimensions.height_2d);
    set_ivec3_uniform("texelDimensions3D",
                      s_sim_params.texture_dimensions.width_3d,
                      s_sim_params.texture_dimensions.height_3d,
                      s_sim_params.texture_dimensions.length_3d);
    set_vec3_uniform("dr", 1.0, 1.0, 1.0);
    set_vec3_uniform("dimensions3D",
                     s_sim_params.texture_dimensions.width_3d,
                     s_sim_params.texture_dimensions.height_3d,
                     s_sim_params.texture_dimensions.length_3d);
    draw_unbind_quad();
    bind_quad(s_frames.gradient, s_programs.gradient);
    set_sampler2D_uniform("tex", s_frames.draw);
    set_sampler2D_uniform("boundaryMaskTex", s_frames.boundary_mask);
    set_int_uniform("index", 3);
    set_ivec2_uniform("texelDimensions2D",
                      s_sim_params.texture_dimensions.width_2d,
                      s_sim_params.texture_dimensions.height_2d);
    set_ivec3_uniform("texelDimensions3D",
                      s_sim_params.texture_dimensions.width_3d,
                      s_sim_params.texture_dimensions.height_3d,
                      s_sim_params.texture_dimensions.length_3d);
    set_vec3_uniform("dr", 1.0, 1.0, 1.0);
    set_vec3_uniform("dimensions3D", 
                     s_sim_params.texture_dimensions.width_3d,
                     s_sim_params.texture_dimensions.height_3d,
                     s_sim_params.texture_dimensions.length_3d);
    draw_unbind_quad();
}

static struct DVec3 normalize(struct DVec3 r) {
    double norm = sqrt(r.x*r.x + r.y*r.y + r.z*r.z);
    struct DVec3 v = {.x=r.x/norm, .y=r.y/norm, .z=r.z/norm};
    return v;
}

double length(struct DVec3 r) {
    return sqrt(r.x*r.x + r.y*r.y + r.z*r.z);
}

static struct DVec4 quaternion_multiply(struct DVec4 q1, struct DVec4 q2) {
    struct DVec4 q3 = {
        .w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z,
        .x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y,
        .y = q1.w*q2.y + q1.y*q2.w + q1.z*q2.x - q1.x*q2.z,
        .z = q1.w*q2.z + q1.z*q2.w + q1.x*q2.y - q1.y*q2.x,
    };
    return q3;
}

static struct DVec3 cross_product(struct DVec3 r1, struct DVec3 r2) {
    struct DVec3 r3 = {
        .x = r1.y*r2.z - r1.z*r2.y,
        .y = - r1.x*r2.z + r1.z*r2.x,
        .z = r1.x*r2.y - r1.y*r2.x,
    };
    return r3;
}

static struct DVec4
rotation_axis_to_quaternion(double angle, struct DVec3 axis) {
    double norm = sqrt(axis.x*axis.x + axis.y*axis.y + axis.z*axis.z);
    for (int i = 0; i < 3; i++) {
        axis.ind[i] = axis.ind[i]/norm;
    }
    double c = cos(angle/2.0);
    double s = sin(angle/2.0);
    struct DVec4 res = {.x=s*axis.x, .y=s*axis.y, .z=s*axis.z, .w=c};
    return res;
}


void render(const struct RenderParams *render_params) {
    if (render_params->user_use &&
        (render_params->user_dx != 0.0 || render_params->user_dy != 0.0)) {
        double angle = 4.0*sqrt(
            render_params->user_dx*render_params->user_dx
             + render_params->user_dy*render_params->user_dy);
        struct DVec3 to_camera = {.x=0.0, .y=0.0, .z=1.0};
        struct DVec3 vel = {.x=render_params->user_dx,
                            .y=render_params->user_dy,
                            .z=0.0};
        struct DVec3 unorm_axis = cross_product(vel, to_camera);
        struct DVec3 axis = normalize(unorm_axis);
        if (length(axis) > (1.0 - 1e-10)
             && length(axis) < (1.0 + 1e-10)) {
            struct DVec4 q_axis
                     = rotation_axis_to_quaternion(angle, axis);
            if (render_params->mode0 == 1) {
                struct DVec4 tmp
                    = quaternion_multiply(s_sim_params.rotation, q_axis);
                s_sim_params.rotation.x = tmp.x;
                s_sim_params.rotation.y = tmp.y;
                s_sim_params.rotation.z = tmp.z;
                s_sim_params.rotation.w = tmp.w;
            } else if (render_params->mode0 == -1) {
                struct DVec4 tmp
                    = quaternion_multiply(s_sim_params.rotation2, q_axis);
                s_sim_params.rotation2.x = tmp.x;
                s_sim_params.rotation2.y = tmp.y;
                s_sim_params.rotation2.z = tmp.z;
                s_sim_params.rotation2.w = tmp.w;
            }
        }

    }
    s_sim_params.scale = (float)render_params->user_scroll;

    glViewport(0, 0, 
               s_sim_params.render_texture_dimensions.width_2d,
               s_sim_params.render_texture_dimensions.height_2d);
    
    struct VertexParam vertex_params[2] = {
        {.name="uvIndex", .size=4, .type=GL_FLOAT, .normalized=GL_FALSE,
         .stride=4*sizeof(float), .offset=0},
    };

    bind_frame(s_frames.sub_view1, s_programs.sample_vol);
    set_vertex_attributes(vertex_params, 1);
    set_float_uniform("scale", 1.0/s_sim_params.scale);
    set_vec4_uniform("rotation", s_sim_params.rotation.ind[0],
                     s_sim_params.rotation.ind[1],
                     s_sim_params.rotation.ind[2],
                     s_sim_params.rotation.ind[3]);
    set_ivec2_uniform("renderTexelDimensions2D",
                      s_sim_params.render_texture_dimensions.width_2d,
                      s_sim_params.render_texture_dimensions.height_2d);
    set_ivec3_uniform("renderTexelDimensions3D",
                      s_sim_params.render_texture_dimensions.width_3d,
                      s_sim_params.render_texture_dimensions.height_3d,
                      s_sim_params.render_texture_dimensions.length_3d);
    set_ivec2_uniform("texelDimensions2D",
                      s_sim_params.texture_dimensions.width_2d,
                      s_sim_params.texture_dimensions.height_2d);
    set_ivec3_uniform("texelDimensions3D",
                      s_sim_params.texture_dimensions.width_3d,
                      s_sim_params.texture_dimensions.height_3d,
                      s_sim_params.texture_dimensions.length_3d);
    set_sampler2D_uniform("tex", s_frames.draw);
    glDrawElements(GL_TRIANGLES, s_sizeof_elements, GL_UNSIGNED_INT, 0);
    unbind();

    bind_frame(s_frames.sub_view2, s_programs.sample_vol);
    set_vertex_attributes(vertex_params, 1);
    set_float_uniform("scale", 1.0/s_sim_params.scale);
    set_vec4_uniform("rotation", s_sim_params.rotation.ind[0],
                     s_sim_params.rotation.ind[1],
                     s_sim_params.rotation.ind[2],
                     s_sim_params.rotation.ind[3]);
    set_ivec2_uniform("renderTexelDimensions2D",
                      s_sim_params.render_texture_dimensions.width_2d,
                      s_sim_params.render_texture_dimensions.height_2d);
    set_ivec3_uniform("renderTexelDimensions3D",
                      s_sim_params.render_texture_dimensions.width_3d,
                      s_sim_params.render_texture_dimensions.height_3d,
                      s_sim_params.render_texture_dimensions.length_3d);
    set_ivec2_uniform("texelDimensions2D",
                      s_sim_params.texture_dimensions.width_2d,
                      s_sim_params.texture_dimensions.height_2d);
    set_ivec3_uniform("texelDimensions3D",
                      s_sim_params.texture_dimensions.width_3d,
                      s_sim_params.texture_dimensions.height_3d,
                      s_sim_params.texture_dimensions.length_3d);
    set_sampler2D_uniform("tex", s_frames.gradient);
    glDrawElements(GL_TRIANGLES, s_sizeof_elements, GL_UNSIGNED_INT, 0);
    unbind();

   // glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bind_frame(s_frames.sub_view3, s_programs.show_vol);
    set_vertex_attributes(vertex_params, 1);
    set_vec4_uniform("rotation", s_sim_params.rotation.ind[0],
                     s_sim_params.rotation.ind[1],
                     s_sim_params.rotation.ind[2],
                     s_sim_params.rotation.ind[3]);
    set_vec4_uniform("debugRotation", s_sim_params.rotation2.ind[0],
                     s_sim_params.rotation2.ind[1],
                     s_sim_params.rotation2.ind[2],
                     s_sim_params.rotation2.ind[3]);
    set_float_uniform("scale", 1.0);
    // set_float_uniform("scale", s_sim_params.scale);
    set_ivec2_uniform("texelDimensions2D",
                      s_sim_params.render_texture_dimensions.width_2d,
                      s_sim_params.render_texture_dimensions.height_2d);
    set_ivec3_uniform("texelDimensions3D",
                      s_sim_params.render_texture_dimensions.width_3d,
                      s_sim_params.render_texture_dimensions.height_3d,
                      s_sim_params.render_texture_dimensions.length_3d);
    set_sampler2D_uniform("gradientTex", s_frames.sub_view2);
    set_sampler2D_uniform("densityTex", s_frames.sub_view1);
    glDrawElements(GL_TRIANGLES, s_sizeof_elements, GL_UNSIGNED_INT, 0);
    unbind();

    glClear(GL_COLOR_BUFFER_BIT);
    // glClear(GL_DEPTH_BUFFER_BIT);
    glClear(GL_STENCIL_BUFFER_BIT);
    // glDisable(GL_DEPTH_TEST);


    glViewport(0, 0,
               s_sim_params.view_width,
              s_sim_params.view_height);
    bind_quad(s_frames.main_view, s_programs.copy);
    if (render_params->mode1 == 1)
        set_sampler2D_uniform("tex", s_frames.sub_view1);
    else if (render_params->mode1 == 2)
        set_sampler2D_uniform("tex", s_frames.sub_view2);
    else if (render_params->mode1 == 3)
        set_sampler2D_uniform("tex", s_frames.sub_view3);
    draw_unbind_quad();
}
