#include <stdio.h>
#include <stdlib.h>
#include "gl_wrappers/gl_wrappers.h"

#ifndef _SERIALIZE_
#define _SERIALIZE_

#ifdef __cplusplus
extern "C" {
#endif

void serialize(const char *fname,
               int quad_id, const struct TextureParams *tex_params);

int deserialize(const char *fname,
                struct TextureParams *tex_params);

#ifdef __cplusplus
}
#endif

#endif
