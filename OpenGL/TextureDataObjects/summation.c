#include "summation.h"
#include "frames_stacks.h"
#include "unary_ops.h"
#include "gl_wrappers/gl_wrappers.h"
#include <GLES3/gl3.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

static struct {
    int is_initialized;
    GLuint scale_program;
    GLuint squared_program;
    GLuint reduce_2d_to_1d_program;
    GLuint reduce_to_single_channel_program;
} s_summation_programs = {0,};


void init_summation_programs() {
    if (!s_summation_programs.is_initialized) {
        s_summation_programs.scale_program
             = make_quad_program("./shaders/scale.frag");
        s_summation_programs.squared_program
             = make_quad_program("./shaders/squared.frag");
        s_summation_programs.reduce_2d_to_1d_program
             = make_quad_program("./shaders/reduce-2d-to-1d.frag");
        s_summation_programs.reduce_to_single_channel_program
            = make_quad_program("./shaders/reduce-to-single-channel.frag");
    }
}

static GLuint to_base(int sized) {
    switch(sized) {
    case GL_RGBA32F: case GL_RGBA32I: case GL_RGBA32UI: case GL_RGBA16F:
    case GL_RGBA16I: case GL_RGBA16UI: case GL_RGBA8I: case GL_RGBA8UI:
        return GL_RGBA;
    case GL_RGB32F: case GL_RGB32I: case GL_RGB32UI: case GL_RGB16F:
    case GL_RGB16I: case GL_RGB16UI: case GL_RGB8I: case GL_RGB8UI:
    case GL_RGB8:
        return GL_RGB;
    case GL_RG32F: case GL_RG32I: case GL_RG32UI: case GL_RG16F:
    case GL_RG16I: case GL_RG16UI: case GL_RG8I: case GL_RG8UI:
        return GL_RG;
    case GL_R32F: case GL_R32I: case GL_R32UI: case GL_R16F:
    case GL_R16I: case GL_R16UI: case GL_R8: case GL_R8UI:
        return GL_RED;
    }
    return -1;
}
static GLuint to_type(int sized) {
    switch(sized) {
    case GL_RGBA32F: case GL_RGB32F: case GL_RG32F: case GL_R32F:
        return GL_FLOAT;
    case GL_RGBA32I: case GL_RGB32I: case GL_RG32I: case GL_R32I:
        return GL_INT;
    case GL_RGBA32UI: case GL_RGB32UI: case GL_RG32UI: case GL_R32UI:
        return GL_UNSIGNED_INT;
    case GL_RGBA16F: case GL_RGB16F: case GL_RG16F: case GL_R16F:
        return GL_HALF_FLOAT;
    case GL_RGBA16I: case GL_RGB16I: case GL_RG16I: case GL_R16I:
        return GL_SHORT;
    case GL_RGBA16UI: case GL_RGB16UI: case GL_RG16UI: case GL_R16UI:
        return GL_UNSIGNED_SHORT;
    case GL_RGBA8: case GL_RGB8: case GL_RG8: case GL_R8:
        return GL_BYTE;
    case GL_RGBA8UI: case GL_RGB8UI: case GL_RG8UI: case GL_R8UI:
        return GL_UNSIGNED_BYTE;
    }
    return -1;
}

#define sum_arr(sum_val, arr, width, height)\
for (int i = 0; i < height; i++) {\
for (int j = 0; j < width; j++) {\
sum_val += arr[width*i + j];\
}\
}

#define sum_vec_arr(sum_val, vec_len, arr, width, height)\
for (int i = 0; i < height; i++) {\
for (int j = 0; j < width; j++) {\
for (int k = 0; k < vec_len; k++) {\
sum_val.ind[k] += arr[width*i + j].ind[k];\
}\
}\
}


static struct PixelData
sum_for_loop(frame_id quad_id,
             const struct TextureParams *tex_params) {
    int format = tex_params->format;
    void *buff = calloc(sizeof(void *),
                        tex_params->width*tex_params->height*4);
    struct PixelData res = {.bytes={0,}};
    if (format == GL_RGBA32F || format == GL_RGBA16F) {
        get_quad_array(quad_id, tex_params, (struct Vec4 *)buff);
        sum_vec_arr(res.as_dvec4, 4, ((struct Vec4 *)buff),
                    tex_params->width, tex_params->height);
    } else if (format == GL_RGB32F || format == GL_RGB16F) {
        get_quad_array(quad_id, tex_params, (struct DVec3 *)buff);
        sum_vec_arr(res.as_dvec4, 3, ((struct DVec3 *)buff),
                    tex_params->width, tex_params->height);
    } else if (format == GL_RG32F || format == GL_RG16F) {
        get_quad_array(quad_id, tex_params, (struct DVec2 *)buff);
        sum_vec_arr(res.as_dvec4, 2, ((struct DVec2 *)buff),
                    tex_params->width, tex_params->height);
    } else if (format == GL_R32F || format == GL_R16F) {
        get_quad_array(quad_id, tex_params, (float *)buff);
        sum_arr(res.as_double, ((float *)buff),
                tex_params->width, tex_params->height);
        // sum_vec_arr(res.as_dvec4, 1, ((struct DVec4 *)buff),
        //             tex_params->width, tex_params->height);
    }
    free(buff);
    return res;
    /*switch(format) {
        case GL_RGBA32F:
        case GL_RGB32F:
        case GL_RG32F:
        case GL_R32F:
        case GL_RGBA16F:
        case GL_RGB16F:
        case GL_RG16F:
        case GL_R16F:
        case GL_RGBA32I:
        case GL_RGB32I:
        case GL_RG32I:
        case GL_R32I:
        case GL_RGBA8:
        case GL_RGB8:
        case GL_RG8:
        case GL_R8:
            break;/
    }*/
}

static void
reduce_2d_to_1d_for_loop(frame_id dst, frame_id src,
                         const struct TextureParams *dst_tex_params,
                         const struct TextureParams *src_tex_params) {
    int dst_w = dst_tex_params->width;
    int dst_h = dst_tex_params->height;
    int src_w = src_tex_params->width;
    int src_h = src_tex_params->height;
    // sum_over_index uses the glsl index convention
    // (0 accesses horizontally, 1 vertically).
    int sum_over_index;
    if (dst_w == src_h || dst_h == src_h) {
        // Sum over the horizontal or 0th since the dimension along vertical
        // (src_h to dst_h) remains the same.
        sum_over_index = 0;
    } else if (dst_w == src_w || dst_h == src_w) {
        sum_over_index = 1;
    }
    glViewport(0, 0, dst_w, dst_h);
    bind_quad(dst, s_summation_programs.reduce_2d_to_1d_program);
    set_sampler2D_uniform("tex", src);
    set_int_uniform("sumOverIndex", sum_over_index);
    set_int_uniform("srcWidth", src_w);
    set_int_uniform("srcHeight", src_h);
    #define TRUE 1
    #define FALSE 0
    set_int_uniform("dstIsRow", (dst_h == 1)? TRUE: FALSE);
    #undef TRUE
    #undef FALSE
    draw_unbind_quad();
    glViewport(0, 0, src_w, src_h);
}

void reduce_2d_to_1d(frame_id dst, frame_id src,
                     const struct TextureParams *dst_tex_params,
                     const struct TextureParams *src_tex_params) {
    int dst_w = dst_tex_params->width;
    int dst_h = dst_tex_params->height;
    int src_w = src_tex_params->width;
    int src_h = src_tex_params->height;
    int sum_over_index;
    if (dst_w == src_h || dst_h == src_h) {
        sum_over_index = 0;
    } else if (dst_w == src_w || dst_h == src_w) {
        sum_over_index = 1;
    } else {
        fprintf(stderr, "Dimensions for sum reduction"
                " do not match.\n");
        return;
    }
    if (dst_w != 1 && dst_h != 1) {
        fprintf(stderr, "Destination texture must be 1D.\n");
        return;
    }
    if ((src_h % 2 == 0 || src_w % 2 == 0) &&
        (src_w > 3 && src_h > 3)) {
        struct TextureParams new_tex_params = {};
        copy_tex_params(&new_tex_params, src_tex_params);
        new_tex_params.width = (sum_over_index == 0)? src_w/2 : src_w;
        new_tex_params.height = (sum_over_index == 1)? src_h/2: src_h;
        frame_id new_frame = activate_frame(&new_tex_params);
        int tmp_viewport[4] = {0,};
        glGetIntegerv(GL_VIEWPORT, tmp_viewport);
        glViewport(0, 0, new_tex_params.width, new_tex_params.height);
        bind_quad(new_frame, s_summation_programs.scale_program);
        set_sampler2D_uniform("tex", src);
        set_float_uniform("scale", 2.0);
        draw_unbind_quad();
        reduce_2d_to_1d(dst, new_frame, dst_tex_params, &new_tex_params);
        deactivate_frame(&new_tex_params, new_frame);
        glViewport(tmp_viewport[0], tmp_viewport[1],
                   tmp_viewport[2], tmp_viewport[3]);
    } else {
        reduce_2d_to_1d_for_loop(dst, src,
                                 dst_tex_params, src_tex_params);
    }
}

void reduce_to_single_channel(frame_id dst, frame_id src,
                              int src_size) {
    bind_quad(dst,
              s_summation_programs.reduce_to_single_channel_program);
    set_int_uniform("size", src_size);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

struct PixelData sum(frame_id quad_id,
                     const struct TextureParams *tex_params) {
    // TODO: if using an older version of OpenGL default to this line:
    // return sum_for_loop(quad_id, tex_params);
    struct TextureParams new_tex_params = {};
    memcpy(&new_tex_params, tex_params,
           sizeof(struct TextureParams));
    /* #ifdef __EMSCRIPTEN__
    return sum_for_loop(quad_id, tex_params);
    #endif*/
    if (((tex_params->width % 2 == 0) || (tex_params->height % 2 == 0)) &&
        (tex_params->width > 3 && tex_params->height > 3)) {
        if (tex_params->width % 2 == 0)
            new_tex_params.width /= 2;
        if (tex_params->height % 2 == 0)
            new_tex_params.height /= 2;
        frame_id new_frame = activate_frame(&new_tex_params);
        glViewport(0, 0, new_tex_params.width, new_tex_params.height);
        bind_quad(new_frame, s_summation_programs.scale_program);
        set_sampler2D_uniform("tex", quad_id);
        if (tex_params->width % 2 == 0 && tex_params->height % 2 == 0)
            set_float_uniform("scale", 4.0);
        else
            set_float_uniform("scale", 2.0);
        draw_unbind_quad();
        struct PixelData res = sum(new_frame, &new_tex_params);
        glViewport(0, 0, tex_params->width, tex_params->height);
        deactivate_frame(&new_tex_params, new_frame);
        return res;
    } else {
        return sum_for_loop(quad_id, tex_params);
    }
}


struct PixelData norm_squared(frame_id src,
                              const struct TextureParams *tex_params) {
    GLuint type = to_type(tex_params->format);
    #ifndef __EMSCRIPTEN__
    int tmp_format = GL_R32F;
    if (type == GL_INT || type == GL_SHORT || type == GL_BYTE)
        tmp_format = GL_R32I;
    else if (type == GL_UNSIGNED_INT || type == GL_UNSIGNED_SHORT
             || type == GL_UNSIGNED_BYTE)
        tmp_format = GL_R32UI;
    #else
    int tmp_format = GL_RGBA32F;
    if (type == GL_INT || type == GL_SHORT || type == GL_BYTE)
        tmp_format = GL_RGBA32I;
    else if (type == GL_UNSIGNED_INT || type == GL_UNSIGNED_SHORT
             || type == GL_UNSIGNED_BYTE)
        tmp_format = GL_RGBA32UI;
    #endif
    struct TextureParams tmp_tex_params = {
        .format=tmp_format,
        .width=tex_params->width, .height=tex_params->height,
        .generate_mipmap=tex_params->generate_mipmap,
        .wrap_s=tex_params->wrap_s, .wrap_t=tex_params->wrap_t,
        .min_filter=tex_params->min_filter, .mag_filter=tex_params->mag_filter,
    };
    frame_id tmp_frame = activate_frame(&tmp_tex_params);
    bind_quad(tmp_frame, s_summation_programs.squared_program);
    set_sampler2D_uniform("tex", src);
    GLuint base_val = to_base(tex_params->format);
    int size = 0;
    if (base_val == GL_RGBA) size = 4;
    if (base_val == GL_RGB) size = 3;
    if (base_val == GL_RG) size = 2;
    if (base_val == GL_RED) size = 1;
    // printf("size: %d\n", size);
    set_int_uniform("size", size);
    draw_unbind_quad();
    struct PixelData dat = sum(tmp_frame, &tmp_tex_params);
    // printf("Pixel data value as float: %g\n", dat.as_float);
    deactivate_frame(&tmp_tex_params, tmp_frame);
    return dat;
}

/*
static struct PixelData
norm_squared_recursive(frame_id src, struct TextureParams *tex_params) {
    int width = tex_params->width;
    int height = tex_params->height;
    if (width < 4 && height < 4) {

    }
    if ((width % 2) == 0 && (height & 2) == 0) {
    } else if (width % 2 == 0) {

    } else if (height % 2 == 0) {

    } else {

    }
}*/

#undef sum_arr
#undef sum_vec_arr
