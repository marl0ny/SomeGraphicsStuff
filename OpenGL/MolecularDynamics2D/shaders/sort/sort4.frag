/* Sort on each consecutive size 4 subblock of a texture.
This is used to implement the base case of the recursive 
bitonic sort network algorithm.

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

void sort2(inout vec4 high, inout vec4 low, vec4 in1, vec4 in2) {
    if (compareVal(in1) > compareVal(in2)) {
        high = in1;
        low = in2;
        return;
    }
    high = in2;
    low = in1;
}

void bitonicSort4(
    inout vec4 res0, inout vec4 res1, inout vec4 res2, inout vec4 res3,
    vec4 in0, vec4 in1, vec4 in2, vec4 in3, bool sortHigh2Low
) {
    vec4 h1, low1, h2, low2;
    sort2(h1, low1, in0, in2);
    sort2(h2, low2, in1, in3);
    if (!sortHigh2Low) {
        sort2(res1, res0, low1, low2);
        sort2(res3, res2, h1, h2);
    } else {
        sort2(res0, res1, h1, h2);
        sort2(res2, res3, low1, low2);
    }
}

void main() {
    int index = indexFromTextureCoordinates(UV);
    int indexDivBlock = int(floor(float(index)/4.0));
    int blockPos = 4*indexDivBlock;
    int i0 = blockPos;
    int i1 = blockPos + 1;
    int i2 = blockPos + 2;
    int i3 = blockPos + 3;
    vec4 in0 = texture2D(tex, toTextureCoordinates(i0));
    vec4 in1 = texture2D(tex, toTextureCoordinates(i1));
    vec4 in2 = texture2D(tex, toTextureCoordinates(i2));
    vec4 in3 = texture2D(tex, toTextureCoordinates(i3));
    vec4 high1, low1, high2, low2;
    sort2(high1, low1, in0, in1);
    sort2(high2, low2, in2, in3);
    // If in ascending order.
    vec4 a0, a1, a2, a3;
    // Else if the order is descending.
    vec4 d0, d1, d2, d3;
    bitonicSort4(a0, a1, a2, a3, low1, high1, high2, low2, false);
    bitonicSort4(d0, d1, d2, d3, low1, high1, high2, low2, true);
    bool sortLow2High = mod(float(indexDivBlock), 2.0) == 0.0;
    if (index == i0)
        fragColor = sortLow2High? a0: d0;
    else if (index == i1)
        fragColor = sortLow2High? a1: d1;
    else if (index == i2)
        fragColor = sortLow2High? a2: d2;
    else if (index == i3)
        fragColor = sortLow2High? a3: d3;
}