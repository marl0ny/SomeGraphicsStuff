#include "gl_wrappers/gl_wrappers.h"

#ifndef _SUMMATION_
#define _SUMMATION_

#ifdef __cplusplus
extern "C" {
#endif

void init_summation_programs();

void reduce_2d_to_1d(frame_id dst, frame_id src,
                     const struct TextureParams *dst_tex_params,
                     const struct TextureParams *src_tex_params);

void reduce_to_single_channel(frame_id ds, frame_id src,
                              int src_size);

struct PixelData norm_squared(frame_id src,
                              const struct TextureParams *tex_params);

struct PixelData sum(frame_id src,
                     const struct TextureParams *tex_params);



#ifdef __cplusplus
}
#endif

#endif
