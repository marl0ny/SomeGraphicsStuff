#include "gl_wrappers/gl_wrappers.h"

#ifndef _FFT_
#define _FFT_

#ifdef __cplusplus
extern "C" {
#endif

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
