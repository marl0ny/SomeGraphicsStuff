/* This shader is used to implement part of the recursive step of 
bitonic sort. The texture is divided into consecutive equally-sized
subblocks, where the recursive step is performed individually on each 
of these subblocks. The size of these subblock is determined through 
uniform values, where at each iteration of the recursive step, the size of
this subblock is divided by two. Here the texels of the lower half of each
of these subblocks are shifted to be placed along the even indices local
to these subblocks, and the upper half are placed along the odd indices. 
Then each contiguous even-odd pair are compared: if the bitonic sort is to 
ultimately be done in ascending order, the texels are swapped to ensure that
the even index contains the smaller value while the odd index contains the 
larger; if in descending order, the even index contains the larger value.

This only partially performs the recursive step of this bitonic sort;
a second stage is required to finish this step.

References:

Kipfer P., Westermann R, "Improved GPU Sorting,"
in GPU Gems 2, ch 46.
https://developer.nvidia.com/gpugems/gpugems2/
part-vi-simulation-and-numerical-algorithms/
chapter-46-improved-gpu-sorting

"Bitonic Sort," in Wikipedia.
https://en.wikipedia.org/wiki/Bitonic_sorter

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

void sort2(inout vec4 high, inout vec4 low, vec4 in1, vec4 in2) {
    if (in1[0] > in2[1]) {
        high = in1;
        low = in2;
    } else {
        high = in2;
        low = in1;
    }
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
    sort2(eHigh, eLow, e, eNext);
    sort2(oHigh, oLow, o, oLast);
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