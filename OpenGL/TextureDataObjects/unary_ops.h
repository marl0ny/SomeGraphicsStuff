#include "gl_wrappers/gl_wrappers.h"

#ifndef _UNARY_OPS_
#define _UNARY_OPS_

#ifdef __cplusplus
extern "C" {
#endif

struct Laplacian2DParams {
    double dx;
    double dy;
    double width;
    double height;
    int order_of_accuracy;
};

struct Grad2DParams {
    double dx, dy;
    double width, height;
    int staggered;
    int order_of_accuracy;
};

void init_unary_ops_programs();

void tex_copy(frame_id dst, frame_id src);

void tex_zero(frame_id dst);

void tex_scale(frame_id dst, frame_id src, double val);

void tex_laplacian2D(frame_id dst, frame_id src,
                     struct Laplacian2DParams *params);

void tex_ddx(frame_id dst, frame_id src,
             const struct Grad2DParams *params);

void tex_ddy(frame_id dst, frame_id src,
             const struct Grad2DParams *params);

void tex_bilerp(frame_id dst,
                const struct Vec4 *w00, const struct Vec4 *w01,
                const struct Vec4 *w10, const struct Vec4 *w11);

void tex_swizzle(frame_id dst, frame_id src,
                 int c0, int c1, int c2, int c3);

void tex_roll(frame_id dst, frame_id src, const struct Vec2 *translate_uv);

void tex_conj(frame_id dst, frame_id src);

void tex_cos(frame_id dst, frame_id src);

void tex_sin(frame_id dst, frame_id src);

void tex_exp(frame_id dst, frame_id src);

void tex_log(frame_id dst, frame_id src);

void tex_tan(frame_id dst, frame_id src);

void tex_sqrt(frame_id dst, frame_id src);

void tex_complex_cos(frame_id dst, frame_id src);

void tex_complex_sin(frame_id dst, frame_id src);

void tex_complex_exp(frame_id dst, frame_id src);

void tex_complex_sqrt(frame_id dst, frame_id src);

void tex_substitute_float(frame_id dst, frame_id src,
                          float old_val, float new_val);

#ifdef __cplusplus
}
#endif

#endif
