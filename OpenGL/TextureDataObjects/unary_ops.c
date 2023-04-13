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
    GLuint gradient;
    GLuint laplacian;
    GLuint bilinear;
    GLuint float_substitute;
} s_unary_ops_programs = {0, };

void init_unary_ops_programs() {
    if (!s_unary_ops_programs.is_initialized) {
        s_unary_ops_programs.copy = make_quad_program("./shaders/copy.frag");
        s_unary_ops_programs.zero = make_quad_program("./shaders/zero.frag");
        s_unary_ops_programs.scale
             = make_quad_program("./shaders/scale.frag");
        s_unary_ops_programs.functions
            = make_quad_program("./shaders/funcs.frag");
        s_unary_ops_programs.gradient
            = make_quad_program("./shaders/gradient.frag");
        s_unary_ops_programs.laplacian
            = make_quad_program("./shaders/laplacian.frag");
        s_unary_ops_programs.bilinear
            = make_quad_program("./shaders/bilinear.frag");
        s_unary_ops_programs.swizzle
            = make_quad_program("./shaders/swizzle.frag");
        s_unary_ops_programs.conj
            = make_quad_program("./shaders/conj.frag");
        s_unary_ops_programs.roll
            = make_quad_program("./shaders/roll.frag");
        s_unary_ops_programs.float_substitute
            = make_quad_program("./shaders/float-substitute.frag");
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

enum {
    R_COS=0, R_SIN=1, R_EXP=2, R_LOG=3, R_TAN=4, R_SQRT=5,
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
