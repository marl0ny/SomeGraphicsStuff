/* This shader is used to implement the final part of
the recursive step of bitonic sort.

For each subblock of the input texture, those texels
with even-valued indices within this subblock
are placed at the lower half of this subblock,
while those of odd indices are moved to the upper
half.

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

void main() {
    int index = indexFromTextureCoordinates(UV);
    int blockIndex = int(mod(float(index), float(blockSize)));
    int blockPos = int(float(blockSize)*floor(float(index)/float(blockSize)));
    vec4 e = texture2D(tex, 
        toTextureCoordinates(blockPos + blockIndex*2));
    vec4 o = texture2D(tex, 
        toTextureCoordinates(blockPos + 2*(blockIndex - blockSize/2) + 1));
    if (blockIndex < blockSize/2)
        fragColor = e; 
    else
        fragColor = o;

}
