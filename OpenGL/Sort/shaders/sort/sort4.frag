/* Sort on each consecutive size 4 subblock of a texture.
This is used to implement the base case of the recursive 
bitonic sort network algorithm.

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

void bitonicSort4(
    inout vec4 res0, inout vec4 res1, inout vec4 res2, inout vec4 res3,
    vec4 in0, vec4 in1, vec4 in2, vec4 in3, bool sortHigh2Low
) {
    vec4 h1, low1, h2, low2;
    sort2(h1, low1, in0, in2);
    sort2(h2, low2, in1, in3);
    if (!sortHigh2Low) {
        res0 = (low1[0] < low2[0])? low1: low2;
        res1 = (low2[0] > low1[0])? low2: low1;
        res2 = (h1[0] < h2[0])? h1: h2;
        res3 = (h2[0] > h1[0])? h2: h1;
    } else {
        res0 = (h2[0] > h1[0])? h2: h1;
        res1 = (h1[0] < h2[0])? h1: h2;
        res2 = (low2[0] > low1[0])? low2: low1;
        res3 = (low1[0] < low2[0])? low1: low2;
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
    bool sortLow2High = abs(mod(float(indexDivBlock), 2.0)) < 1e-30;
    if (index == i0)
        fragColor = sortLow2High? a0: d0;
    else if (index == i1)
        fragColor = sortLow2High? a1: d1;
    else if (index == i2)
        fragColor = sortLow2High? a2: d2;
    else if (index == i3)
        fragColor = sortLow2High? a3: d3;
}