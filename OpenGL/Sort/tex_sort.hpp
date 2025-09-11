/* Implementation of sorting that utilizes GLSL
shaders to sort the texel units of a texture, using
a bitonic sorting network. 

References:

Kipfer P., Westermann R, "Improved GPU Sorting,"
in GPU Gems 2, ch 46.
https://developer.nvidia.com/gpugems/gpugems2/
part-vi-simulation-and-numerical-algorithms/
chapter-46-improved-gpu-sorting

"Bitonic Sort," in Wikipedia.
https://en.wikipedia.org/wiki/Bitonic_sorter

*/
#include "gl_wrappers.hpp"

#ifndef _SORT_GL_
#define _SORT_GL_

namespace sort {

class Sort;

class Programs {
    unsigned int sort4;
    struct {
        unsigned int size4;
        unsigned int part1;
        unsigned int part2;
    } bitonic_sort;
    Programs();
    friend class Sort;
};

class Frames {
    TextureParams sort_texture;
    Quad tmp;
    Quad sort_iter[2];
    Frames(int w, int h);
    void reset_dimensions(int w, int h);
    friend class Sort;
};

class Sort {
    Programs m_programs;
    Frames m_frames;
    public:
    /* The value of texture_width*texture_height
    MUST BE A POWER OF TWO, or else this will not work properly.*/
    Sort(int texture_width, int texture_height);
    Sort(IVec2 tex_dimensions2d);
    /* The dimensions of both dst and src must be the same,
    and dst.width(), dst.height() MUST BE EQUAL to the width
    and height parameters entered in the constructor, or else
    bad things will happen. */
    void operator()(Quad &dst, const Quad &src);
    void bitonic(Quad &dst, const Quad &src);
    void bitonic(Quad &dst, const Quad &src, int size);
    void reset_dimensions(int tex_width, int tex_height);
    void reset_dimensions(IVec2 tex_dimensions2d);
};

}

#endif