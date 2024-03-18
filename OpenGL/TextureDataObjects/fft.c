#include "fft.h"
#include "frames_stacks.h"
#include "unary_ops.h"
#include <stdio.h>

#define boolean int
#define TRUE 1
#define FALSE 0

#define ORIENTATION_0 0
#define ORIENTATION_1 1
#define ORIENTATION_2 2


static boolean power2_leq_4096(int val) {
    switch(val) {
    case 1: case 2: case 4: case 8: case 16: case 32: case 64: case 128:
    case 256: case 512: case 1024: case 2048: case 4096:
        return TRUE;
    default:
        return FALSE;
    }
}

static struct {
    int is_initialized;
    GLuint fft;
    GLuint rev_bit_sort2;
    GLuint fftshift;
    GLuint fft_3d;
    GLuint rev_bit_sort2_3d;
    GLuint fftshift_3d;
} s_fft_programs = {0, };


void init_fft_programs() {
    if (!s_fft_programs.is_initialized) {
        s_fft_programs.fft = make_quad_program("./shaders/fft-iter.frag");
        s_fft_programs.rev_bit_sort2
            = make_quad_program("./shaders/rev-bit-sort2.frag");
        s_fft_programs.fftshift
            = make_quad_program("./shaders/fftshift.frag");
        s_fft_programs.fft_3d
            = make_quad_program("./shaders/fft-iter3d.frag");
        s_fft_programs.rev_bit_sort2_3d
            = make_quad_program("./shaders/rev-bit-sort2-3d.frag");
        s_fft_programs.fftshift_3d
            = make_quad_program("./shaders/fftshift3d.frag");
    }
    s_fft_programs.is_initialized = TRUE;
}


static void swap2(frame_id *f1, frame_id *f2) {
    frame_id tmp = *f2;
    *f2 = *f1;
    *f1 = tmp;
}

static void rev_bit_sort2(int rev_bit_sort2_program,
                          frame_id initial, frame_id dest,
                          int width, int height) {
    bind_quad(dest, rev_bit_sort2_program);
    set_sampler2D_uniform("tex", initial);
    set_int_uniform("width", width);
    set_int_uniform("height", height);
    draw_unbind_quad();
}

static frame_id fft_iter(int fft_iter_program,
                         frame_id fft_iter1, frame_id fft_iter2,
                         int size, int is_vertical) {
    frame_id iter2[2] = {fft_iter1, fft_iter2};
    for (float block_size = 2.0; block_size <= size; block_size *= 2) {
        bind_quad(iter2[1], fft_iter_program);
        set_sampler2D_uniform("tex", iter2[0]);
        set_int_uniform("isVertical", is_vertical);
        set_float_uniform("blockSize", block_size/(float)size);
        set_float_uniform("angleSign", -1.0);
        set_float_uniform("size", (float)size);
        set_float_uniform("scale", 1.0);
        draw_unbind_quad();
        swap2(&iter2[0], &iter2[1]);
    }
    return iter2[0];
}

static frame_id ifft_iter(int fft_iter_program,
                          frame_id fft_iter1, frame_id fft_iter2,
                          int size, int is_vertical) {
    frame_id iter2[2] = {fft_iter1, fft_iter2};
    for (float block_size = 2.0; block_size <= size; block_size *= 2) {
        bind_quad(iter2[1], fft_iter_program);
        set_sampler2D_uniform("tex",  iter2[0]);
        set_int_uniform("isVertical", is_vertical);
        set_float_uniform("blockSize", block_size/(float)size);
        set_float_uniform("angleSign", 1.0);
        set_float_uniform("size", (float)size);
        set_float_uniform("scale",
                          (block_size == size)? 1.0/(float)size: 1.0);
        draw_unbind_quad();
        swap2(&iter2[0], &iter2[1]);
    }
    return iter2[0];
}

static void tex_fft_helper(frame_id dst, frame_id src,
                           boolean is_inverse,
                           const struct TextureParams *tex_params) {
    int width = tex_params->width;
    int height = tex_params->height;
    if (!power2_leq_4096(width) || !power2_leq_4096(height)) {
        fprintf(stderr,
                "Width and height must be a power of two and "
                "less than or equal to 4096.\n");
        return;
    }
    int iter_frame1 = activate_frame(tex_params);
    int iter_frame2 = activate_frame(tex_params);
    rev_bit_sort2(s_fft_programs.rev_bit_sort2,
                  src, iter_frame1, width, height);
    frame_id (*iter_func) 
        (int, frame_id, frame_id, int, int) = (is_inverse)? ifft_iter: fft_iter;
    int res = iter_func(s_fft_programs.fft,
                        iter_frame1, iter_frame2, width, FALSE);
    if (res == iter_frame1) {
        res = iter_func(s_fft_programs.fft,
                        iter_frame1, iter_frame2, height, TRUE);
    } else {
        res = iter_func(s_fft_programs.fft,
                        iter_frame2, iter_frame1, height, TRUE);
    }
    tex_copy(dst, res);
    deactivate_frame(tex_params, iter_frame1);
    deactivate_frame(tex_params, iter_frame2);
}

void tex_fft(frame_id dst, frame_id src,
             const struct TextureParams *tex_params) {
    tex_fft_helper(dst, src, FALSE, tex_params);
}

void tex_ifft(frame_id dst, frame_id src,
              const struct TextureParams *tex_params) {
    tex_fft_helper(dst, src, TRUE, tex_params);
}

void tex_fftshift(frame_id dst, frame_id src,
                  const struct TextureParams *tex_params) {
    bind_quad(dst, s_fft_programs.fftshift);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

static void rev_bit_sort2_3d(int rev_bit_sort2_3d_program,
                             frame_id initial, frame_id dest,
                             const struct IVec2 *texture_dimensions,
                             const struct IVec3 *dimensions_3d) {
    bind_quad(dest, rev_bit_sort2_3d_program);
    set_sampler2D_uniform("tex", initial);
    set_ivec2_uniform("texelDimensions2D", 
                      texture_dimensions->x, texture_dimensions->y);
    set_ivec3_uniform("texelDimensions3D",
                      dimensions_3d->x, dimensions_3d->y, dimensions_3d->z);
    draw_unbind_quad();
}


static frame_id fft_iter3d(int fft_iter_3d_program,
                           frame_id fft_iter1, frame_id fft_iter2,
                           int size, int orientation,
                           const struct IVec2 *texture_dimensions,
                           const struct IVec3 *dimensions_3d) {
    frame_id iter2[2] = {fft_iter1, fft_iter2};
    for (float block_size = 2.0; block_size <= size; block_size *= 2) {
        bind_quad(iter2[1], fft_iter_3d_program);
        set_sampler2D_uniform("tex", iter2[0]);
        set_float_uniform("blockSize", block_size/(float)size);
        set_int_uniform("orientation", orientation);
        set_float_uniform("angleSign", -1.0);
        set_float_uniform("size", (float)size);
        set_float_uniform("scale", 1.0);
        set_ivec2_uniform("texelDimensions2D", 
                          texture_dimensions->x, texture_dimensions->y);
        set_ivec3_uniform("texelDimensions3D", 
                          dimensions_3d->x, 
                          dimensions_3d->y, 
                          dimensions_3d->z);
        draw_unbind_quad();
        swap2(&iter2[0], &iter2[1]);
    }
    return iter2[0];
}

static frame_id ifft_iter3d(int fft_iter_3d_program,
                            frame_id fft_iter1, frame_id fft_iter2,
                            int size, int orientation,
                            const struct IVec2 *texture_dimensions,
                            const struct IVec3 *dimensions_3d) {
    frame_id iter2[2] = {fft_iter1, fft_iter2};
    for (float block_size = 2.0; block_size <= size; block_size *= 2) {
        bind_quad(iter2[1], fft_iter_3d_program);
        set_sampler2D_uniform("tex", iter2[0]);
        set_float_uniform("blockSize", block_size/(float)size);
        set_int_uniform("orientation", orientation);
        set_float_uniform("angleSign", 1.0);
        set_float_uniform("size", (float)size);
        set_float_uniform("scale", 
            (block_size == size)? 1.0/(float)size: 1.0);
        set_ivec2_uniform("texelDimensions2D", 
                          texture_dimensions->x, texture_dimensions->y);
        set_ivec3_uniform("texelDimensions3D", 
                          dimensions_3d->x, 
                          dimensions_3d->y, 
                          dimensions_3d->z);
        draw_unbind_quad();
        swap2(&iter2[0], &iter2[1]);
    }
    return iter2[0];
}

static void tex_fft3d_helper(frame_id dst, frame_id src,
                             boolean is_inverse,
                             const struct IVec3 *dimensions_3d,
                             const struct TextureParams *tex_params) {
    struct IVec2 texture_dimensions = {
        {{.x=tex_params->width, .y=tex_params->height}}
    };
    if (!power2_leq_4096(dimensions_3d->x) 
        || !power2_leq_4096(dimensions_3d->y) 
        || !power2_leq_4096(dimensions_3d->z)) {
        fprintf(stderr,
                "Each dimension must be a power of two and "
                "less than or equal to 4096.\n");
        return;
    }
    int iter_frame1 = activate_frame(tex_params);
    int iter_frame2 = activate_frame(tex_params);
    rev_bit_sort2_3d(s_fft_programs.rev_bit_sort2_3d,
                     src, iter_frame1,
                     &texture_dimensions,
                     dimensions_3d);
    frame_id (* iter_func)(
        int, frame_id, frame_id, int, int, const struct IVec2 *, const struct IVec3 *)
         = (is_inverse)? ifft_iter3d: fft_iter3d;
    // Do each fft along the x axis
    int res = iter_func(s_fft_programs.fft_3d, 
                        iter_frame1, iter_frame2,
                        dimensions_3d->ind[0], ORIENTATION_0, 
                        &texture_dimensions, dimensions_3d);
    // y axis
    res = iter_func(s_fft_programs.fft_3d, 
                    (res == iter_frame1)? iter_frame1: iter_frame2, 
                    (res == iter_frame1)? iter_frame2: iter_frame1,
                    dimensions_3d->ind[1], ORIENTATION_1, 
                    &texture_dimensions, dimensions_3d);
    // z axis
    res = iter_func(s_fft_programs.fft_3d, 
                    (res == iter_frame1)? iter_frame1: iter_frame2, 
                    (res == iter_frame1)? iter_frame2: iter_frame1,
                    dimensions_3d->ind[2], ORIENTATION_2, 
                    &texture_dimensions, dimensions_3d);
    tex_copy(dst, res);
    deactivate_frame(tex_params, iter_frame1);
    deactivate_frame(tex_params, iter_frame2);
}

void tex_fft3d(frame_id dst, frame_id src,
               const struct IVec3 *dimensions_3d,
               const struct TextureParams *tex_params) {
    tex_fft3d_helper(dst, src, FALSE, dimensions_3d, tex_params);
}

void tex_ifft3d(frame_id dst, frame_id src,
                const struct IVec3 *dimensions_3d,
                const struct TextureParams *tex_params) {
    tex_fft3d_helper(dst, src, TRUE, dimensions_3d, tex_params);

}

void tex_fftshift3d(frame_id dst, frame_id src,
                    const struct IVec3 *dimensions_3d,
                    const struct TextureParams *tex_params) {
    int iter_frame = activate_frame(tex_params);
    for (int orientation = 0; orientation < 3; orientation++) {
        switch (orientation) {
            case ORIENTATION_0:
            bind_quad(dst, s_fft_programs.fftshift_3d);
            set_sampler2D_uniform("tex", src);
            break;
            case ORIENTATION_1:
            bind_quad(iter_frame, s_fft_programs.fftshift_3d);
            set_sampler2D_uniform("tex", dst);
            break;
            case ORIENTATION_2:
            bind_quad(dst, s_fft_programs.fftshift_3d);
            set_sampler2D_uniform("tex", iter_frame);
            break;
        }
        set_ivec2_uniform(
            "texelDimensions2D", 
            tex_params->width, tex_params->height);
        set_ivec3_uniform(
            "texelDimensions3D", 
            dimensions_3d->x, dimensions_3d->y, dimensions_3d->z);
        set_int_uniform("orientation", orientation);
        draw_unbind_quad();
    }
    deactivate_frame(tex_params, iter_frame);
}
