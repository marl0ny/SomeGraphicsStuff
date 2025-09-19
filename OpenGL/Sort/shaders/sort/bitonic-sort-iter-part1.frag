/* This shader is used to implement part of the recursive step of 
bitonic sort. The texture is divided into consecutive equally-sized
subblocks, where the recursive step is performed individually on each 
of these subblocks. The size of these subblock is determined through 
uniform values, where at each iteration of the recursive step, the size of
this subblock is divided by two. Here the texels of the lower half of each
of these subblocks are rearranged to be along the even indices local
to these subblocks, and the upper half are placed along the odd indices. 
Then each contiguous even-odd pair are compared: if the bitonic sort is to 
ultimately be done in ascending order, the texels are swapped to ensure that
the even index contains the smaller value while the odd index contains the 
larger; if in descending order, the even index contains the larger value.

This only partially performs the recursive step of this bitonic sort;
a second stage is required to finish this step.

References:

P. Kipfer, R. Westermann, "Improved GPU Sorting,"
in GPU Gems 2, ch 46.
Available: https://developer.nvidia.com/gpugems/gpugems2/
part-vi-simulation-and-numerical-algorithms/
chapter-46-improved-gpu-sorting

"Bitonic Sort." Wikipedia.com.
Available: https://en.wikipedia.org/wiki/Bitonic_sorter

*/
#if (__VERSION__ >= 330) || (defined(GL_ES) && __VERSION__ >= 300)
#define texture2D texture
#else
#define texture texture2D
#endif

#if (__VERSION__ > 120) || defined(GL_ES)
precision highp float;
#endif
    
#if __VERSION__ <= 120
varying vec2 UV;
#define fragColor gl_FragColor
#else
in vec2 UV;
out vec4 fragColor;
#endif

uniform sampler2D tex;
uniform ivec2 texDimensions2D;
uniform int blockSize;
uniform int flipOrderSize;

uniform int comparisonMethod;
const int COMPARE_R = 0;
const int COMPARE_G = 1;
const int COMPARE_B = 2;
const int COMPARE_A = 3;
/* These next comparison methods
use the distance of the texel's xy, xyz, or xyzw
vectors with that provided by the compareStartPoint
uniform. */
uniform vec4 compareStartPoint;
const int COMPARE_XY_DIST = 4;
const int COMPARE_XYZ_DIST = 5;
const int COMPARE_XYZW_DIST = 6;
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
const int COMPARE_2D_X_MAJOR_CELL_IND = 7;
const int COMPARE_3D_X_MAJOR_CELL_IND = 8;
uniform ivec3 gridOfCellsDimensions;
uniform vec3 gridOfCellsOrigin;
uniform vec3 cellDimensions;

int indexFromTextureCoordinates(vec2 UV) {
    int width = texDimensions2D[0];
    int height = texDimensions2D[1];
    int rowCount = int(UV.y*float(height) - 0.5);
    int columnPosition = int(UV.x*float(width) - 0.5); 
    return columnPosition + rowCount*width;
}

vec2 toTextureCoordinates(int index) {
    int width = texDimensions2D[0];
    int height = texDimensions2D[1];
    float x = mod(float(index), float(width));
    float y = floor((float(index))/float(width));
    return vec2(
        (x + 0.5)/float(width), (y + 0.5)/float(height));
}

float compareVal(vec4 texel) {
    if (comparisonMethod == COMPARE_R) {
        return texel[0];
    } else if (comparisonMethod == COMPARE_G) {
        return texel[1];
    } else if (comparisonMethod == COMPARE_B) {
        return texel[2];
    } else if (comparisonMethod == COMPARE_A) {
        return texel[3];
    } else if (comparisonMethod == COMPARE_XY_DIST) {
        return dot(
            texel.xy - compareStartPoint.xy, 
            texel.xy - compareStartPoint.xy);
    } else if (comparisonMethod == COMPARE_XYZ_DIST) {
        return dot(
            texel.xyz - compareStartPoint.xyz,
            texel.xyz - compareStartPoint.xyz);
    } else if (comparisonMethod == COMPARE_XYZW_DIST) {
        return dot(texel - compareStartPoint, texel - compareStartPoint);
    } else if (comparisonMethod == COMPARE_2D_X_MAJOR_CELL_IND) {
        float x = texel.x, y = texel.y;
        float cellXInd = floor((x - gridOfCellsOrigin.x)/cellDimensions.x);
        float cellYInd = floor((y - gridOfCellsOrigin.y)/cellDimensions.y);
        return cellXInd + cellYInd*float(gridOfCellsDimensions[0]);
    } else if (comparisonMethod == COMPARE_3D_X_MAJOR_CELL_IND) {
        float x = texel.x, y = texel.y, z = texel.z;
        float cellXInd = floor((x - gridOfCellsOrigin.x)/cellDimensions.x);
        float cellYInd = floor((y - gridOfCellsOrigin.y)/cellDimensions.y);
        float cellZInd = floor((z - gridOfCellsOrigin.z)/cellDimensions.z);
        return cellXInd 
            + cellYInd*float(gridOfCellsDimensions[0]) 
            + cellZInd
            *float(gridOfCellsDimensions[0])*float(gridOfCellsDimensions[1]);
    }
}

void sort2A(inout vec4 high, inout vec4 low, vec4 in1, vec4 in2) {
    if (compareVal(in1) >= compareVal(in2)) {
        high = in1;
        low = in2;
        return;
    }
    high = in2;
    low = in1;
}

void sort2B(inout vec4 high, inout vec4 low, vec4 in1, vec4 in2) {
    if (compareVal(in1) > compareVal(in2)) {
        high = in1;
        low = in2;
        return;
    }
    high = in2;
    low = in1;
}

bool isDivisibleBy2(int val) {
    return mod(float(val), 2.0) == 0.0;
}

void main() {
    int index = indexFromTextureCoordinates(UV);
    // If this value is divisible by two, go from low to high
    // else go from high to low.
    int orderIndex = int(floor(float(index)/float(flipOrderSize)));
    // Index inside a block
    int blockIndex = int(mod(float(index), float(blockSize)));
    // Index offset of the block
    int blockPos = int(float(blockSize)*floor(float(index)/float(blockSize)));
    // For an even value of blockIndex, sample blockIndex/2
    vec4 e = texture2D(tex, 
        toTextureCoordinates(blockPos + blockIndex/2));
    // If blockIndex is even, then the next value must be odd,
    // so for the next value sample blockSize/2 + blockIndex/2
    vec4 eNext = texture2D(tex, 
        toTextureCoordinates(blockPos + blockIndex/2 + blockSize/2));
    // For an odd value of blockIndex,
    // sample (blockIndex - 1)/2 + blockSize/2
    vec4 o = texture2D(tex, 
        toTextureCoordinates(blockPos + (blockIndex - 1)/2 + blockSize/2));
    vec4 oLast = texture2D(tex, 
        toTextureCoordinates(blockPos + (blockIndex - 1)/2));
    vec4 eHigh, eLow, oHigh, oLow;
    sort2A(eHigh, eLow, e, eNext);
    sort2B(oHigh, oLow, o, oLast);
    // When orderIndex % 2 == 0, go from low to high

    if (isDivisibleBy2(orderIndex) && isDivisibleBy2(blockIndex))
        fragColor = eLow;
    else if (isDivisibleBy2(orderIndex) && !isDivisibleBy2(blockIndex))
        fragColor = oHigh;
    // When indexDivBlock % 2 == 1, go from hight to low
    else if (!isDivisibleBy2(orderIndex) && isDivisibleBy2(blockIndex))
        fragColor = eHigh;
    else if (!isDivisibleBy2(orderIndex) && !isDivisibleBy2(blockIndex))
        fragColor = oLow;
}