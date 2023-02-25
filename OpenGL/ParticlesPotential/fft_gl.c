#include <stdlib.h>
// #include "fft_gl.h"
#include "gl_wrappers/gl_wrappers.h"


void swap2(frame_id *f1, frame_id *f2) {
    frame_id tmp = *f2;
    *f2 = *f1;
    *f1 = tmp;
}

void rev_bit_sort2(int rev_bit_sort2_program,
                   frame_id initial, frame_id dest,
                   int width, int height) {
    bind_quad(dest, rev_bit_sort2_program);
    set_sampler2D_uniform("tex", initial);
    set_int_uniform("width", width);
    set_int_uniform("height", height);
    // print_user_defined_uniforms();
    draw_unbind_quad();
}

frame_id fft_iter(int fft_iter_program,
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

frame_id ifft_iter(int fft_iter_program,
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
        set_float_uniform("scale", (block_size == size)? 1.0/(float)size: 1.0);
        draw_unbind_quad();
        swap2(&iter2[0], &iter2[1]);
    }
    return iter2[0];
}
