#include "bin_ops.h"

#define TRUE 1
#define FALSE 0


static struct {
    int is_initialized;
    GLuint tex, tex_float, tex_vec2, tex_vec3, tex_vec4;
} s_bin_ops_programs = {0, };

void init_bin_ops_programs() {
    if (!s_bin_ops_programs.is_initialized) {
        s_bin_ops_programs.tex
            = make_quad_program("./shaders/tex-bin-ops.frag");
        s_bin_ops_programs.tex_float
            = make_quad_program("./shaders/tex-float-bin-ops.frag");
        s_bin_ops_programs.tex_vec2
            = make_quad_program("./shaders/tex-vec2-bin-ops.frag");
        s_bin_ops_programs.tex_vec3
            = make_quad_program("./shaders/tex-vec3-bin-ops.frag");
        s_bin_ops_programs.tex_vec4
            = make_quad_program("./shaders/tex-vec4-bin-ops.frag");
        s_bin_ops_programs.is_initialized = TRUE;
    }
}

enum {
    DIV=2, MUL=3, ADD=4, SUB=5, COMPLEX_DIV=12, COMPLEX_MUL=13
};

void tex_tex_add_tex(frame_id dst, frame_id x, frame_id y) {
    bind_quad(dst, s_bin_ops_programs.tex);
    set_int_uniform("opType", ADD);
    set_sampler2D_uniform("texL", x);
    set_sampler2D_uniform("texR", y);
    draw_unbind_quad();
}

void tex_tex_sub_tex(frame_id dst, frame_id x, frame_id y) {
    bind_quad(dst, s_bin_ops_programs.tex);
    set_int_uniform("opType", SUB);
    set_sampler2D_uniform("texL", x);
    set_sampler2D_uniform("texR", y);
    draw_unbind_quad();
}

void tex_tex_mul_tex(frame_id dst, frame_id x, frame_id y) {
    bind_quad(dst, s_bin_ops_programs.tex);
    set_int_uniform("opType", MUL);
    set_sampler2D_uniform("texL", x);
    set_sampler2D_uniform("texR", y);
    draw_unbind_quad();
}

void tex_tex_div_tex(frame_id dst, frame_id x, frame_id y) {
    bind_quad(dst, s_bin_ops_programs.tex);
    set_int_uniform("opType", DIV);
    set_sampler2D_uniform("texL", x);
    set_sampler2D_uniform("texR", y);
    draw_unbind_quad();
}

void tex_tex_complex_mul_tex(frame_id dst, frame_id x, frame_id y) {
    bind_quad(dst, s_bin_ops_programs.tex);
    set_int_uniform("opType", COMPLEX_MUL);
    set_sampler2D_uniform("texL", x);
    set_sampler2D_uniform("texR", y);
    draw_unbind_quad();
}

void tex_tex_complex_div_tex(frame_id dst, frame_id x, frame_id y) {
    bind_quad(dst, s_bin_ops_programs.tex);
    set_int_uniform("opType", COMPLEX_DIV);
    set_sampler2D_uniform("texL", x);
    set_sampler2D_uniform("texR", y);
    draw_unbind_quad();
}

void tex_tex_add_float(frame_id dst, frame_id x, float y) {
    bind_quad(dst, s_bin_ops_programs.tex_float);
    set_int_uniform("opType", ADD);
    set_float_uniform("val", y);
    set_int_uniform("texOnLeft", TRUE);
    set_sampler2D_uniform("tex", x);
    draw_unbind_quad();
}

void tex_float_add_tex(frame_id dst, float x, frame_id y) {
    tex_tex_add_float(dst, y, x);
}

void tex_tex_sub_float(frame_id dst, frame_id x, float y) {
    bind_quad(dst, s_bin_ops_programs.tex_float);
    set_int_uniform("opType", SUB);
    set_float_uniform("val", y);
    set_int_uniform("texOnLeft", TRUE);
    set_sampler2D_uniform("tex", x);
    draw_unbind_quad();
}

void tex_float_sub_tex(frame_id dst, float x, frame_id y) {
    bind_quad(dst, s_bin_ops_programs.tex_float);
    set_int_uniform("opType", SUB);
    set_float_uniform("val", x);
    set_int_uniform("texOnLeft", FALSE);
    set_sampler2D_uniform("tex", y);
    draw_unbind_quad();
}

void tex_tex_mul_float(frame_id dst, frame_id x, float y) {
    bind_quad(dst, s_bin_ops_programs.tex_float);
    set_int_uniform("opType", MUL);
    set_float_uniform("val", y);
    set_int_uniform("texOnLeft", TRUE);
    set_sampler2D_uniform("tex", x);
    draw_unbind_quad();
}

void tex_float_mul_tex(frame_id dst, float x, frame_id y) {
    tex_tex_mul_float(dst, y, x);
}

void tex_tex_div_float(frame_id dst, frame_id x, float y) {
    bind_quad(dst, s_bin_ops_programs.tex_float);
    set_int_uniform("opType", DIV);
    set_float_uniform("val", y);
    set_int_uniform("texOnLeft", TRUE);
    set_sampler2D_uniform("tex", x);
    draw_unbind_quad();
}

void tex_float_div_tex(frame_id dst, float x, frame_id y) {
    bind_quad(dst, s_bin_ops_programs.tex_float);
    set_int_uniform("opType", DIV);
    set_float_uniform("val", x);
    set_int_uniform("texOnLeft", FALSE);
    set_sampler2D_uniform("tex", y);
    draw_unbind_quad();
}

void tex_tex_complex_mul_float(frame_id dst, frame_id x, float y) {
    bind_quad(dst, s_bin_ops_programs.tex_float);
    set_int_uniform("opType", COMPLEX_MUL);
    set_float_uniform("val", y);
    set_int_uniform("texOnLeft", TRUE);
    set_sampler2D_uniform("tex", x);
    draw_unbind_quad();
}

void tex_float_complex_mul_tex(frame_id dst, float x, frame_id y) {
    tex_tex_complex_mul_float(dst, y, x);
}

void tex_tex_complex_div_float(frame_id dst, frame_id x, float y) {
    bind_quad(dst, s_bin_ops_programs.tex_float);
    set_int_uniform("opType", COMPLEX_DIV);
    set_float_uniform("val", y);
    set_int_uniform("texOnLeft", TRUE);
    set_sampler2D_uniform("tex", x);
    draw_unbind_quad();
}

void tex_float_complex_div_tex(frame_id dst, float x, frame_id y) {
    bind_quad(dst, s_bin_ops_programs.tex_float);
    set_int_uniform("opType", COMPLEX_DIV);
    set_float_uniform("val", x);
    set_int_uniform("texOnLeft", FALSE);
    set_sampler2D_uniform("tex", y);
    draw_unbind_quad();
}

void tex_tex_add_vec2(frame_id dst, frame_id x, const struct Vec2 *y) {
    bind_quad(dst, s_bin_ops_programs.tex_vec2);
    set_int_uniform("opType", ADD);
    set_int_uniform("texOnLeft", TRUE);
    set_sampler2D_uniform("tex", x);
    set_vec2_uniform("val", y->ind[0], y->ind[1]);
    draw_unbind_quad();
}

void tex_vec2_add_tex(frame_id dst, const struct Vec2 *x, frame_id y) {
    tex_tex_add_vec2(dst, y, x);
}

void tex_tex_sub_vec2(frame_id dst, frame_id x, const struct Vec2 *y) {
    bind_quad(dst, s_bin_ops_programs.tex_vec2);
    set_int_uniform("opType", SUB);
    set_int_uniform("texOnLeft", TRUE);
    set_sampler2D_uniform("tex", x);
    set_vec2_uniform("val", y->ind[0], y->ind[1]);
    draw_unbind_quad();
}

void tex_vec2_sub_tex(frame_id dst, const struct Vec2 *x, frame_id y) {
    bind_quad(dst, s_bin_ops_programs.tex_vec2);
    set_int_uniform("opType", SUB);
    set_int_uniform("texOnLeft", FALSE);
    set_sampler2D_uniform("tex", y);
    set_vec2_uniform("val", x->ind[0], x->ind[1]);
    draw_unbind_quad();
}

void tex_tex_mul_vec2(frame_id dst, frame_id x, const struct Vec2 *y) {
    bind_quad(dst, s_bin_ops_programs.tex_vec2);
    set_int_uniform("opType", MUL);
    set_int_uniform("texOnLeft", TRUE);
    set_sampler2D_uniform("tex", x);
    set_vec2_uniform("val", y->ind[0], y->ind[1]);
    draw_unbind_quad();
}

void tex_vec2_mul_tex(frame_id dst, const struct Vec2 *x, frame_id y) {
    tex_tex_mul_vec2(dst, y, x);
}

void tex_tex_div_vec2(frame_id dst, frame_id x, const struct Vec2 *y) {
    bind_quad(dst, s_bin_ops_programs.tex_vec2);
    set_int_uniform("opType", DIV);
    set_int_uniform("texOnLeft", TRUE);
    set_sampler2D_uniform("tex", x);
    set_vec2_uniform("val", y->ind[0], y->ind[1]);
    draw_unbind_quad();
}

void tex_vec2_div_tex(frame_id dst, const struct Vec2 *x, frame_id y) {
    bind_quad(dst, s_bin_ops_programs.tex_vec2);
    set_int_uniform("opType", DIV);
    set_int_uniform("texOnLeft", FALSE);
    set_sampler2D_uniform("tex", y);
    set_vec2_uniform("val", x->ind[0], x->ind[1]);
    draw_unbind_quad();
}

void tex_tex_complex_mul_vec2(frame_id dst, frame_id x, const struct Vec2 *y) {
    bind_quad(dst, s_bin_ops_programs.tex_vec2);
    set_int_uniform("opType", COMPLEX_MUL);
    set_int_uniform("texOnLeft", TRUE);
    set_sampler2D_uniform("tex", x);
    set_vec2_uniform("val", y->ind[0], y->ind[1]);
    draw_unbind_quad();
}

void tex_vec2_complex_mul_tex(frame_id dst, const struct Vec2 *x, frame_id y) {
    tex_tex_complex_mul_vec2(dst, y, x);
}

void tex_tex_complex_div_vec2(frame_id dst, frame_id x, const struct Vec2 *y) {
    bind_quad(dst, s_bin_ops_programs.tex_vec2);
    set_int_uniform("opType", COMPLEX_DIV);
    set_int_uniform("texOnLeft", TRUE);
    set_sampler2D_uniform("tex", x);
    set_vec2_uniform("val", y->ind[0], y->ind[1]);
    draw_unbind_quad();

}

void tex_vec2_complex_div_tex(frame_id dst, const struct Vec2 *x, frame_id y) {
    bind_quad(dst, s_bin_ops_programs.tex_vec2);
    set_int_uniform("opType", COMPLEX_DIV);
    set_int_uniform("texOnLeft", FALSE);
    set_sampler2D_uniform("tex", y);
    set_vec2_uniform("val", x->ind[0], x->ind[1]);
    draw_unbind_quad();

}

#undef TRUE
#undef FALSE
