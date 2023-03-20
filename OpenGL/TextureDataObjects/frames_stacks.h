#include "gl_wrappers/gl_wrappers.h"

#ifndef _FRAMES_STACKS_
#define _FRAMES_STACKS_

#ifdef __cplusplus
extern "C" {
#endif

frame_id activate_frame(const struct TextureParams *tex_params);

void deactivate_frame(const struct TextureParams *tex_params,
                      frame_id id_of_frame);

#ifdef __cplusplus
}
#endif

#endif
