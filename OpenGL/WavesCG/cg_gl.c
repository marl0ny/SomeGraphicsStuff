#include "cg_gl.h"


static void swap2(frame_id *f1, frame_id *f2) {
    frame_id tmp  = *f1;
    *f1 = *f2;
    *f2 = tmp;
}

static void vec4_transfer(struct Vec4 *z, const struct Vec4 *v) {
    z->x=v->x, z->y=v->y, z->z=v->z, z->w=v->w;
}

struct Vec4 vec4_divide(const struct Vec4 *num, const struct Vec4 *den) {
    struct Vec4 frac = {.x=num->x/den->x, .y=num->y/den->y,
                        .z=num->z/den->z, .w=num->w/den->w};
    return frac;
}

static struct Vec4 dot(frame_id multiply_program,
                       frame_id scale_program,
                       frame_id *sum_quads, int size,
                       frame_id v1, frame_id v2, frame_id v1_v2) {
    bind_quad(v1_v2, multiply_program);
    set_sampler2D_uniform("tex1", v1);
    set_sampler2D_uniform("tex2", v2);
    draw_unbind();
    return texture_reduction_sum(scale_program,
                                 v1_v2, sum_quads,
                                 size);
}

static void add(GLuint add_program, frame_id res,
                const struct Vec4 *s1, frame_id v1,
                const struct Vec4 *s2, frame_id v2) {
    bind_quad(res, add_program);
    set_vec4_uniform("scale1",
                     s1->ind[0], s1->ind[1],
                     s1->ind[2], s1->ind[3]);
    set_sampler2D_uniform("tex1", v1);
    set_vec4_uniform("scale2",
                     s2->ind[0], s2->ind[1],
                     s2->ind[2], s2->ind[3]);
    set_sampler2D_uniform("tex2", v2);
    draw_unbind();
}

static void subtract(GLuint add_program, frame_id res,
                     const struct Vec4 *s1, frame_id v1,
                     const struct Vec4 *s2, frame_id v2) {
    bind_quad(res, add_program);
    set_vec4_uniform("scale1",
                     s1->ind[0], s1->ind[1],
                     s1->ind[2], s1->ind[3]);
    set_sampler2D_uniform("tex1", v1);
    set_vec4_uniform("scale2",
                     -s2->ind[0], -s2->ind[1],
                     -s2->ind[2], -s2->ind[3]);
    set_sampler2D_uniform("tex2", v2);
    draw_unbind();
}

static int continue_iteration(int i, int min_iter, int max_iter,
                              const struct Vec4 *norm_residual2,
                              const struct Vec4 *epsilon) {
    return ((norm_residual2->ind[0] > epsilon->ind[0] ||
             norm_residual2->ind[1] > epsilon->ind[1] ||
             norm_residual2->ind[2] > epsilon->ind[2] ||
             norm_residual2->ind[3] > epsilon->ind[3]) || i < min_iter
             ) && i < max_iter;
}

static void copy(GLuint copy_program, frame_id dst, frame_id src) {
    bind_quad(dst, copy_program);
    set_sampler2D_uniform("tex", src);
    draw_unbind();
}

const struct CGInfo conjugate_gradient(const struct CGController *controls,
                                       void (*transform)(frame_id y,
                                                         void *transform_data,
                                                         frame_id x),
                                       void *transform_data,
                                       frame_id x0, frame_id solution) {
    /*
    References for the conjugate gradient algorithm without preconditioning:
    
    Wikipedia contributors. (2022, September 29). Conjugate Gradient Method.
    In Wikipedia, The Free Encyclopedia.

    Thijssen, J (2007). Numerical Methods. In Computational Physics 
    (pg. 557-604). Cambridge University Press.
    */
    frame_id residual_next = controls->conj_quads[0];
    frame_id residual_last = controls->conj_quads[1];
    frame_id conjugate_next = controls->conj_quads[2];
    frame_id conjugate_last = controls->conj_quads[3];
    frame_id x_next = controls->conj_quads[4];
    frame_id x_last = controls->conj_quads[5];
    frame_id product = controls->conj_quads[6];
    frame_id transform_x0 = controls->conj_quads[7];
    frame_id transform_conjugate_last = controls->conj_quads[7];
    frame_id *sum_quads = controls->sum_quads;
    GLuint scale_program = controls->scale_program;
    GLuint copy_program = controls->copy_program;
    GLuint multiply_program = controls->multiply_program;
    GLuint add_program = controls->add_program;
    struct Vec4 epsilon = controls->epsilon;
    int size = controls->size;
    int min_iter = controls->min_iter, max_iter = controls->max_iter;
    struct Vec4 ones = {.x=1.0, .y=1.0, .z=1.0, .w=1.0};
    int i;
    transform(transform_x0, transform_data, x0);
    // Get the first residual vector
    subtract(add_program, residual_last,
             &ones, solution, &ones, transform_x0);
    // Norm squared value of the initial residual vector
    struct Vec4 norm_residual2 = dot(multiply_program, scale_program,
                                     controls->sum_quads, controls->size,
                                     residual_last, residual_last, product);
    // Initial x vector
    copy(copy_program, x_last, x0);
    // Initial conjugate vector
    copy(copy_program, conjugate_last, residual_last);
    for (i = 0; continue_iteration(i, min_iter, max_iter,
                                   &norm_residual2, &epsilon); i++) {
        // A|conjugate>
        transform(transform_conjugate_last,
                  transform_data, conjugate_last);
        // <conjugate|A|conjugate>
        struct Vec4 conjugate_product
            = dot(multiply_program, scale_program, sum_quads, size,
                  conjugate_last, transform_conjugate_last, product);
        // lambda = <residual|residual> / <conjugate|A|conjugate>
        struct Vec4 lambda
            = vec4_divide(&norm_residual2, &conjugate_product);
        // Get the next x vector
        // |x next> = |x last> + lambda|conjugate>
        add(add_program, x_next,
            &ones, x_last, &lambda, conjugate_last);
        // Compute the next residual vector
        // |residual next> = |residual last> - lambda A|conjugate last>
        subtract(add_program, residual_next,
                 &ones, residual_last, &lambda, transform_conjugate_last);
        // Get the squared norm of the next residual vector
        struct Vec4 norm_residual2_next = dot(multiply_program,
                                              scale_program,
                                              sum_quads, size,
                                              residual_next, residual_next,
                                              product);
        struct Vec4 residual_fraction = vec4_divide(&norm_residual2_next,
                                                    &norm_residual2);
        vec4_transfer(&norm_residual2, &norm_residual2_next);
        // Get the next conjugate vector
        // |conjugate next> = |residual next> +
        //                  |residual next|^2/|residual last|^2|conjugate last>
        add(add_program, conjugate_next,
            &ones, residual_next, &residual_fraction, conjugate_last);
        // Swap things for the next iteration
        swap2(&conjugate_last, &conjugate_next);
        swap2(&residual_last, &residual_next);
        swap2(&x_last, &x_next);
    }
    struct CGInfo cg_info = {.result=x_last,
                             .number_of_iterations=i};
    return cg_info;
}
