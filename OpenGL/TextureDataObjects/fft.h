#include "gl_wrappers/gl_wrappers.h"

#ifndef _FFT_
#define _FFT_

#ifdef __cplusplus
extern "C" {
#endif

/*
Fast fourier transform implementation were most of the computational
effort is intended to be done on the GPU through GLSL and OpenGL.

References:

 - Press W. et al. (1992). Fast Fourier Transform.
   In Numerical Recipes in Fortran 77, chapter 12.

 - Wikipedia contributors. (2021, October 8). Cooley–Tukey FFT algorithm. 
   In Wikipedia, The Free Encyclopedia.
   
 - Weisstein, E. (2021). Fast Fourier Transform. In Wolfram MathWorld.

*/

void init_fft_programs();

void tex_fft(frame_id dst, frame_id src,
             const struct TextureParams *tex_params);

void tex_ifft(frame_id dst, frame_id src,
              const struct TextureParams *tex_params);

void tex_fftshift(frame_id dst, frame_id src,
                  const struct TextureParams *tex_params);

#ifdef __cplusplus
}
#endif

#endif
