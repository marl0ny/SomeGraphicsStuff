#VERSION_NUMBER_PLACEHOLDER

precision highp float;

#if __VERSION__ >= 300
in vec2 UV;
out vec4 fragColor;
#define texture2D texture
#else
#define fragColor gl_FragColor
varying highp vec2 UV;
#endif

#define XY_SLICE 0
#define ZY_SLICE 1
#define XZ_SLICE 2
uniform int orientation;
uniform int slice;

uniform ivec2 sliceTexelDimensions2D;
uniform ivec2 inputTexelDimensions2D;
uniform ivec3 inputTexelDimensions3D;
uniform sampler2D tex;

vec2 to2DTextureCoordinates(vec3 position) {
    int width2D = inputTexelDimensions2D[0];
    int width3D = inputTexelDimensions3D[0];
    int length3D = inputTexelDimensions3D[2];
    float xIndex = float(width3D)*mod(position.x, 1.0);
    float zIndex = float(length3D)*mod(position.z, 1.0);
    float uIndex = floor(zIndex)*float(width3D) + xIndex; 
    return vec2(uIndex/float(width2D), position.y);
}

vec3 to3DTextureCoordinates(vec2 uv) {
    int length3D = inputTexelDimensions3D[2];
    float u = mod(uv[0]*float(length3D), 1.0);
    float v = uv[1];
    float w = (floor(uv[0]*float(length3D)) + 0.5)/float(length3D);
    return vec3(u, v, w);
}

void main() {
    float mx = (float(sliceTexelDimensions2D[0])
                    / float(inputTexelDimensions2D[0]));
    if (orientation == XY_SLICE) {
        // uv2 are the coordinates for sampling the input texture.
        vec2 uv2 = vec2(mx*UV[0] 
                        + float(slice)/float(inputTexelDimensions3D[2]),
                        UV[1]);
        fragColor = texture2D(tex, uv2);
    } else if (orientation == ZY_SLICE) {
        vec2 uv2 = vec2((UV[0] - 0.5/float(sliceTexelDimensions2D[0]))
                         + (float(slice) + 0.5)
                         /float(inputTexelDimensions2D[0]),
                        UV[1]);
        fragColor = texture2D(tex, uv2);
    } else if (orientation == XZ_SLICE) {
        vec2 uv2 = vec2(mx*UV[0] 
                        + (UV[1] - 0.5/float(sliceTexelDimensions2D[0])),
                        (float(slice) + 0.5)/
                        float(inputTexelDimensions2D[1]));
        fragColor = texture2D(tex, uv2);
    }
}