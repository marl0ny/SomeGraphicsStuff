#include "unary_ops.h"

#define TRUE 1
#define FALSE 0

static struct {
    int is_initialized;
    GLuint copy, zero, scale;
    GLuint swizzle;
    GLuint roll;
    GLuint conj;
    GLuint functions;
    GLuint pow;
    GLuint gradient;
    GLuint laplacian;
    GLuint bilinear;
    GLuint trilinear;
    GLuint float_substitute;
    GLuint transpose;
} s_unary_ops_programs = {0, };

void init_unary_ops_programs() {
    if (!s_unary_ops_programs.is_initialized) {
        s_unary_ops_programs.copy 
            = make_quad_program("./shaders/util/copy.frag");
        s_unary_ops_programs.zero 
            = make_quad_program("./shaders/util/zero.frag");
        s_unary_ops_programs.scale
             = make_quad_program("./shaders/util/scale.frag");
        s_unary_ops_programs.functions
            = make_quad_program("./shaders/util/funcs.frag");
        s_unary_ops_programs.pow
            = make_quad_program("./shaders/util/pow.frag");
        s_unary_ops_programs.gradient
            = make_quad_program("./shaders/gradient/gradient.frag");
        s_unary_ops_programs.laplacian
            = make_quad_program("./shaders/laplacian/laplacian.frag");
        s_unary_ops_programs.bilinear
            = make_quad_program("./shaders/util/bilinear.frag");
        s_unary_ops_programs.trilinear
            = make_quad_program("./shaders/util/trilinear.frag");
        s_unary_ops_programs.swizzle
            = make_quad_program("./shaders/util/swizzle.frag");
        s_unary_ops_programs.conj
            = make_quad_program("./shaders/util/conj.frag");
        s_unary_ops_programs.roll
            = make_quad_program("./shaders/util/roll.frag");
        s_unary_ops_programs.float_substitute
            = make_quad_program("./shaders/util/float-substitute.frag");
        s_unary_ops_programs.transpose
            = make_quad_program("./shaders/util/transpose.frag");
        s_unary_ops_programs.is_initialized = TRUE;
    }
}

void tex_copy(frame_id dst, frame_id src) {
    bind_quad(dst, s_unary_ops_programs.copy);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_zero(frame_id dst) {
    bind_quad(dst, s_unary_ops_programs.zero);
    draw_unbind_quad();
}

void tex_scale(frame_id dst, frame_id src, double val) {
    bind_quad(dst, s_unary_ops_programs.scale);
    set_float_uniform("scale", val);
    draw_unbind_quad();
}

void tex_bilerp(frame_id dst,
                const struct Vec4 *w00, const struct Vec4 *w01,
                const struct Vec4 *w10, const struct Vec4 *w11) {
    bind_quad(dst, s_unary_ops_programs.bilinear);
    set_vec4_uniform("w00", w00->x, w00->y, w00->z, w00->w);
    set_vec4_uniform("w10", w10->x, w10->y, w10->z, w10->w);
    set_vec4_uniform("w01", w01->x, w01->y, w01->z, w01->w);
    set_vec4_uniform("w11", w11->x, w11->y, w11->z, w11->w);
    draw_unbind_quad();
}

void tex_trilerp(frame_id dst, const struct IVec3 *texel_dimensions_3d,
                 const struct Vec4 *w000, const struct Vec4 *w010,
                 const struct Vec4 *w100, const struct Vec4 *w110,
                 const struct Vec4 *w001, const struct Vec4 *w011,
                 const struct Vec4 *w101, const struct Vec4 *w111) {
    bind_quad(dst, s_unary_ops_programs.trilinear);
    set_ivec3_uniform("texelDimensions3D",
                      texel_dimensions_3d->x,
                      texel_dimensions_3d->y,
                      texel_dimensions_3d->z);
    // At z = 0
    set_vec4_uniform("w000", w000->x, w000->y, w000->z, w000->w);
    set_vec4_uniform("w100", w100->x, w100->y, w100->z, w100->w);
    set_vec4_uniform("w010", w010->x, w010->y, w010->z, w010->w);
    set_vec4_uniform("w110", w110->x, w110->y, w110->z, w110->w);
    // At z = z_max
    set_vec4_uniform("w001", w001->x, w001->y, w001->z, w001->w);
    set_vec4_uniform("w101", w101->x, w101->y, w101->z, w101->w);
    set_vec4_uniform("w011", w011->x, w011->y, w011->z, w011->w);
    set_vec4_uniform("w111", w111->x, w111->y, w111->z, w111->w);
    draw_unbind_quad();

}

void tex_laplacian2D(frame_id dst, frame_id src,
                     struct Laplacian2DParams *params) {
    bind_quad(dst, s_unary_ops_programs.laplacian);
    set_float_uniform("width", (float)params->width);
    set_float_uniform("height", (float)params->height);
    set_float_uniform("dx", (float)params->dx);
    set_float_uniform("dy", (float)params->dy);
    set_sampler2D_uniform("tex", src);
    set_int_uniform("orderOfAccuracy", params->order_of_accuracy);
    draw_unbind_quad();
}

void tex_ddx(frame_id dst, frame_id src,
             const struct Grad2DParams *params) {
    bind_quad(dst, s_unary_ops_programs.gradient);
    set_float_uniform("width", (float)params->width);
    set_float_uniform("height", (float)params->height);
    set_float_uniform("dx", (float)params->dx);
    set_float_uniform("dy", (float)params->dy);
    set_int_uniform("index", 0);
    set_int_uniform("staggeredMode", params->staggered);
    set_int_uniform("orderOfAccuracy", params->order_of_accuracy);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_ddy(frame_id dst, frame_id src,
             const struct Grad2DParams *params) {
    bind_quad(dst, s_unary_ops_programs.gradient);
    set_float_uniform("width", (float)params->width);
    set_float_uniform("height", (float)params->height);
    set_float_uniform("dx", (float)params->dx);
    set_float_uniform("dy", (float)params->dy);
    set_int_uniform("index", 1);
    set_int_uniform("staggeredMode", params->staggered);
    set_int_uniform("orderOfAccuracy", params->order_of_accuracy);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_swizzle(frame_id dst, frame_id src, int c0, int c1, int c2, int c3) {
    bind_quad(dst, s_unary_ops_programs.swizzle);
    set_sampler2D_uniform("tex", src);
    set_int_uniform("c0", c0);
    set_int_uniform("c1", c1);
    set_int_uniform("c2", c2);
    set_int_uniform("c3", c3);
    draw_unbind_quad();
}

void tex_roll(frame_id dst, frame_id src, const struct Vec2 *translate_uv) {
    bind_quad(dst, s_unary_ops_programs.roll);
    set_sampler2D_uniform("tex", src);
    set_vec2_uniform("translateUV", translate_uv->x, translate_uv->y);
    draw_unbind_quad();
}

void tex_transpose(frame_id dst, frame_id src) {
    bind_quad(dst, s_unary_ops_programs.transpose);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

enum {
    R_COS=0, R_SIN=1, R_EXP=2, R_LOG=3, R_TAN=4, R_SQRT=5, R_POW=6,
    C_COS=64, C_SIN=65, C_EXP=66, C_SQRT=67,
};

void tex_cos(frame_id dst, frame_id src) {
    bind_quad(dst, s_unary_ops_programs.functions);
    set_int_uniform("whichFunction", R_COS);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_sin(frame_id dst, frame_id src) {
    bind_quad(dst, s_unary_ops_programs.functions);
    set_int_uniform("whichFunction", R_SIN);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_exp(frame_id dst, frame_id src) {
    bind_quad(dst, s_unary_ops_programs.functions);
    set_int_uniform("whichFunction", R_EXP);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_log(frame_id dst, frame_id src) {
    bind_quad(dst, s_unary_ops_programs.functions);
    set_int_uniform("whichFunction", R_LOG);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_tan(frame_id dst, frame_id src) {
    bind_quad(dst, s_unary_ops_programs.functions);
    set_int_uniform("whichFunction", R_TAN);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_sqrt(frame_id dst, frame_id src) {
    bind_quad(dst, s_unary_ops_programs.functions);
    set_int_uniform("whichFunction", R_SQRT);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_conj(frame_id dst, frame_id src) {
    bind_quad(dst, s_unary_ops_programs.conj);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_complex_cos(frame_id dst, frame_id src) {
    bind_quad(dst, s_unary_ops_programs.functions);
    set_int_uniform("whichFunction", C_COS);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_complex_sin(frame_id dst, frame_id src) {
    bind_quad(dst, s_unary_ops_programs.functions);
    set_int_uniform("whichFunction", C_SIN);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_complex_exp(frame_id dst, frame_id src) {
    bind_quad(dst, s_unary_ops_programs.functions);
    set_int_uniform("whichFunction", C_EXP);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_complex_sqrt(frame_id dst, frame_id src) {
    bind_quad(dst, s_unary_ops_programs.functions);
    set_int_uniform("whichFunction", C_SQRT);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_pow(frame_id dst, frame_id src, double n) {
    bind_quad(dst, s_unary_ops_programs.pow);
    set_float_uniform("n", (float)n);
    set_sampler2D_uniform("tex", src);
    draw_unbind_quad();
}

void tex_substitute_float(frame_id dst, frame_id src,
                          float old_val, float new_val) {
    bind_quad(dst, s_unary_ops_programs.float_substitute);
    set_sampler2D_uniform("tex", src);
    set_float_uniform("oldVal", old_val);
    set_float_uniform("newVal", new_val);
    draw_unbind_quad();
}

#undef TRUE
#undef FALSE
