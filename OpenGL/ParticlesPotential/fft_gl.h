#include "gl_wrappers/gl_wrappers.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _FFT_GL_
#define _FFT_GL_

void rev_bit_sort2(int rev_bit_sort2_program,
                   frame_id initial, frame_id dest,
                   int width, int height);

frame_id fft_iter(int fft_iter_program,
                  frame_id fft_iter1, frame_id fft_iter2,
                  int size, int is_vertical);

frame_id ifft_iter(int fft_iter_program,
                   frame_id fft_iter1, frame_id fft_iter2,
                   int size, int is_vertical);

#endif

#ifdef __cplusplus
}
#endif

