#include "gl_wrappers/gl_wrappers.h"

#ifndef _BIN_OPS_
#define _BIN_OPS_

#ifdef __cplusplus
extern "C" {
#endif

// Some OpenGL stuff must be initialized first before calling this function.
void init_bin_ops_programs();

// tex = tex op tex
void tex_tex_add_tex(frame_id dst, frame_id x, frame_id y);

void tex_tex_sub_tex(frame_id dst, frame_id x, frame_id y);

void tex_tex_mul_tex(frame_id dst, frame_id x, frame_id y);

void tex_tex_div_tex(frame_id dst, frame_id x, frame_id y);

void tex_tex_complex_mul_tex(frame_id dst, frame_id x, frame_id y);

void tex_tex_complex_div_tex(frame_id dst, frame_id x, frame_id y);

// tex = tex op float
void tex_tex_add_float(frame_id dst, frame_id x, float y);

void tex_float_add_tex(frame_id dst, float x, frame_id y);

void tex_tex_sub_float(frame_id dst, frame_id x, float y);

void tex_float_sub_tex(frame_id dst, float x, frame_id y);

void tex_tex_mul_float(frame_id dst, frame_id x, float y);

void tex_float_mul_tex(frame_id dst, float x, frame_id y);

void tex_tex_div_float(frame_id dst, frame_id x, float y);

void tex_float_div_tex(frame_id dst, float x, frame_id y);

void tex_tex_complex_mul_float(frame_id dst, frame_id x, float y);

void tex_float_complex_mul_tex(frame_id dst, float x, frame_id y);

void tex_tex_complex_div_float(frame_id dst, frame_id x, float y);

void tex_float_complex_div_tex(frame_id dst, float x, frame_id y);

// tex = tex op vec2
void tex_tex_add_vec2(frame_id dst, frame_id x, const struct Vec2 *y);

void tex_vec2_add_tex(frame_id dst, const struct Vec2 *x, frame_id y);

void tex_tex_sub_vec2(frame_id dst, frame_id x, const struct Vec2 *y);

void tex_vec2_sub_tex(frame_id dst, const struct Vec2 *x, frame_id y);

void tex_tex_mul_vec2(frame_id dst, frame_id x, const struct Vec2 *y);

void tex_vec2_mul_tex(frame_id dst, const struct Vec2 *x, frame_id y);

void tex_tex_div_vec2(frame_id dst, frame_id x, const struct Vec2 *y);

void tex_vec2_div_tex(frame_id dst, const struct Vec2 *x, frame_id y);

void tex_tex_complex_mul_vec2(frame_id dst, frame_id x, const struct Vec2 *y);

void tex_vec2_complex_mul_tex(frame_id dst, const struct Vec2 *x, frame_id y);

void tex_tex_complex_div_vec2(frame_id dst, frame_id x, const struct Vec2 *y);

void tex_vec2_complex_div_tex(frame_id dst, const struct Vec2 *x, frame_id y);


// tex = tex op vec3
void tex_tex_add_vec3(frame_id dst, frame_id x, const struct Vec3 *y);

void tex_vec3_add_tex(frame_id dst, const struct Vec3 *x, frame_id y);

void tex_tex_sub_vec3(frame_id dst, frame_id x, const struct Vec3 *y);

void tex_vec3_sub_tex(frame_id dst, const struct Vec3 *x, frame_id y);

void tex_tex_mul_vec3(frame_id dst, frame_id x, const struct Vec3 *y);

void tex_vec3_mul_tex(frame_id dst, const struct Vec3 *x, frame_id y);

void tex_tex_div_vec3(frame_id dst, frame_id x, const struct Vec3 *y);

void tex_vec3_div_tex(frame_id dst, const struct Vec3 *x, frame_id y);

void tex_tex_complex_mul_vec3(frame_id dst, frame_id x, const struct Vec3 *y);

void tex_vec3_complex_mul_tex(frame_id dst, const struct Vec3 *x, frame_id y);

void tex_tex_complex_div_vec3(frame_id dst, frame_id x, const struct Vec3 *y);

void tex_vec3_complex_div_tex(frame_id dst, const struct Vec3 *x, frame_id y);

// tex = tex op vec4
void tex_tex_add_vec4(frame_id dst, frame_id x, const struct Vec4 *y);

void tex_vec4_add_tex(frame_id dst, const struct Vec4 *x, frame_id y);

void tex_tex_sub_vec4(frame_id dst, frame_id x, const struct Vec4 *y);

void tex_vec4_sub_tex(frame_id dst, const struct Vec4 *x, frame_id y);

void tex_tex_mul_vec4(frame_id dst, frame_id x, const struct Vec4 *y);

void tex_vec4_mul_tex(frame_id dst, const struct Vec4 *x, frame_id y);

void tex_tex_div_vec4(frame_id dst, frame_id x, const struct Vec4 *y);

void tex_vec4_div_tex(frame_id dst, const struct Vec4 *x, frame_id y);

void tex_tex_complex_mul_vec4(frame_id dst, frame_id x, const struct Vec4 *y);

void tex_vec4_complex_mul_tex(frame_id dst, const struct Vec4 *x, frame_id y);

void tex_tex_complex_div_vec4(frame_id dst, frame_id x, const struct Vec4 *y);

void tex_vec4_complex_div_tex(frame_id dst, const struct Vec4 *x, frame_id y);

#ifdef __cplusplus
}
#endif

#endif
