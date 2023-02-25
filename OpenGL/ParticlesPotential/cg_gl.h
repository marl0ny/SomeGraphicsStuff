#include "gl_wrappers/gl_wrappers.h"
#include "summation_gl.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifndef _CG_GL_
#define _CG_GL_

struct CGController {
    frame_id *conj_quads; // Textures used in the cg iteration
    frame_id *sum_quads;  // Textures used for performing summations
    int size; // Size of the texture (currently only square
              //                      textures supported)
    GLuint copy_program; // Copy glsl program
    GLuint scale_program; // Scale glsl program
    GLuint add_program, multiply_program; // add and multiply glsl programs
    struct Vec4 epsilon; // minimum size of residual
    int min_iter, max_iter; // minimum and maximum number of iterations.
};

struct CGInfo {
    int number_of_iterations;
    frame_id result;
};

const struct CGInfo conjugate_gradient(const struct CGController *controls,
                                       void (*transform)(frame_id y,
                                                         void *transform_data,
                                                         frame_id x),
                                       void *transform_data,
                                       frame_id x0, frame_id solution);


#endif

#ifdef __cplusplus
}
#endif
