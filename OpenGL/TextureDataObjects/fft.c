#include "fft.h"
#include "frames_stacks.h"
#include "unary_ops.h"

#define TRUE 1
#define FALSE 0

static struct {
    int is_initialized;
    GLuint fft;
    GLuint rev_bit_sort2;
    GLuint fftshift;
    // GLuint functions;
    // GLuint laplacian;
    // GLuint bilinear;
} s_fft_programs = {0, };


void init_fft_programs() {
    if (!s_fft_programs.is_initialized) {
        s_fft_programs.fft = make_quad_program("./shaders/fft-iter.frag");
        s_fft_programs.rev_bit_sort2
            = make_quad_program("./shaders/rev-bit-sort2.frag");
        s_fft_programs.fftshift
            = make_quad_program("./shaders/fftshift.frag");
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

void tex_fft(frame_id dst, frame_id src,
             const struct TextureParams *tex_params) {
    int width = tex_params->width;
    int height = tex_params->height;
    int iter_frame1 = activate_frame(tex_params);
    int iter_frame2 = activate_frame(tex_params);
    rev_bit_sort2(s_fft_programs.rev_bit_sort2,
                  src, iter_frame1, width, height);
    int res = fft_iter(s_fft_programs.fft,
                       iter_frame1, iter_frame2, width, FALSE);
    if (res == iter_frame1) {
        res = fft_iter(s_fft_programs.fft,
                       iter_frame1, iter_frame2, height, TRUE);
    } else {
        res = fft_iter(s_fft_programs.fft,
                       iter_frame2, iter_frame1, height, TRUE);
    }
    tex_copy(dst, res);
    deactivate_frame(tex_params, iter_frame1);
    deactivate_frame(tex_params, iter_frame2);
}

void tex_ifft(frame_id dst, frame_id src,
              const struct TextureParams *tex_params) {
    int width = tex_params->width;
    int height = tex_params->height;
    int iter_frame1 = activate_frame(tex_params);
    int iter_frame2 = activate_frame(tex_params);
    rev_bit_sort2(s_fft_programs.rev_bit_sort2, 
                  src, iter_frame1, width, height);
    int res = ifft_iter(s_fft_programs.fft,
                        iter_frame1, iter_frame2, width, FALSE);
    if (res == iter_frame1) {
        res = ifft_iter(s_fft_programs.fft, 
                        iter_frame1, iter_frame2, height, TRUE);
    } else {
        res = ifft_iter(s_fft_programs.fft, iter_frame2,
                        iter_frame1, height, TRUE);
    }
    tex_copy(dst, res);
    deactivate_frame(tex_params, iter_frame1);
    deactivate_frame(tex_params, iter_frame2);
}

void tex_fftshift(frame_id dst, frame_id src,
                  const struct TextureParams *tex_params) {
    bind_quad(dst, s_fft_programs.fftshift);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

#undef TRUE
#undef FALSE
