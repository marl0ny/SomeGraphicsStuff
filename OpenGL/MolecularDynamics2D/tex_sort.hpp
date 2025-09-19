/* Implementation of sorting that utilizes GLSL
shaders to sort the texel units of a texture, using
a bitonic sorting network. 

References:

P. Kipfer, R. Westermann, "Improved GPU Sorting,"
in GPU Gems 2, ch 46.
Available: https://developer.nvidia.com/gpugems/gpugems2/
part-vi-simulation-and-numerical-algorithms/
chapter-46-improved-gpu-sorting

"Bitonic Sort." Wikipedia.com.
Available: https://en.wikipedia.org/wiki/Bitonic_sorter

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

enum ComparisonMethod {
    COMPARE_R=0, COMPARE_G=1, COMPARE_B=2, COMPARE_A=3,
    /* These next comparison methods
    use the distance of the texel's xy, xyz, or xyzw
    vectors with that provided by the compareStartPoint
    uniform. */
    COMPARE_XY_DIST=4, COMPARE_XYZ_DIST=5, COMPARE_XYZW_DIST=6,
    /* In these next comparison methods,
    it is assumed that the texel's xy or xyz vectors
    are position coordinates in a 2D or 3D space
    that has been partitioned into a uniform grid of cells.
    The "gridOfCellsDimensions", "gridOfCellsOrigin", and
    "cellDimensions" uniforms specify how these grid of
    cells are structured. The cells must ultimately be enumerated
    in a sequential fashion, one after another - i.e. our
    2D or 3D grid of cells are "flattened out" into a 1D description.
    So for those methods with "X_MAJOR" in their name,
    the cells are first enumerated along each row that sits
    parallel with the x-axis, in ascending order of x.
    Once the end of the row is reached, the row above with 
    respect to the y-axis is then counted over in the same
    manner, and so on. */
    COMPARE_2D_X_MAJOR_CELL_IND=7,
    COMPARE_3D_X_MAJOR_CELL_IND=8
};

struct ComparisonParams {
    struct {
        Vec3 origin;
    } distance;
    struct {
        IVec3 grid_dimensions;
        Vec3 grid_origin;
        Vec3 dimensions;
    } cells;
};

class Sort {
    Programs m_programs;
    Frames m_frames;
    ComparisonMethod m_comparison_method;
    ComparisonParams m_params;
    void bitonic(
        Quad &dst, const Quad &src, int size);
    public:
    /* The value of texture_width*texture_height
    MUST BE A POWER OF TWO, or else this will not work properly.*/
    Sort(
        int texture_width, int texture_height,
        ComparisonMethod comparison_method);
    Sort(
        IVec2 tex_dimensions2d, 
        ComparisonMethod comparison_method);
    Sort(
        int texture_width, int texture_height,
        ComparisonMethod comparison_method,
        ComparisonParams params);
    Sort(
        IVec2 tex_dimensions2d, 
        ComparisonMethod comparison_method,
        ComparisonParams params);
    /* The dimensions of both dst and src must be the same,
    and dst.width(), dst.height() MUST BE EQUAL to the width
    and height parameters entered in the constructor, or else
    bad things will happen. */
    void operator()(Quad &dst, const Quad &src);
    void bitonic(Quad &dst, const Quad &src);
    void bitonic4(Quad &dst, const Quad &src);
    void reset_dimensions(int tex_width, int tex_height);
    void reset_dimensions(IVec2 tex_dimensions2d);
};

}

#endif