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

uniform ivec2 inputTexelDimensions2D;
uniform ivec3 inputTexelDimensions3D;
uniform sampler2D tex;


vec2 to2DTextureCoordinates(vec3 uvw) {
    int width2D = inputTexelDimensions2D[0];
    int height2D = inputTexelDimensions2D[1];
    int width3D = inputTexelDimensions3D[0];
    int height3D = inputTexelDimensions3D[1];
    int length3D = inputTexelDimensions3D[2];
    float wStack = float(width2D)/float(width3D);
    // float hStack = float(height2D)/float(height3D);
    float xIndex = float(width3D)*mod(uvw[0], 1.0);
    float yIndex = float(height3D)*mod(uvw[1], 1.0);
    float zIndex = mod(floor(float(length3D)*uvw[2]), float(length3D));
    float uIndex = mod(zIndex, wStack)*float(width3D) + xIndex; 
    float vIndex = floor(zIndex / wStack)*float(height3D) + yIndex; 
    return vec2(uIndex/float(width2D), vIndex/float(height2D));
}

vec3 to3DTextureCoordinates(vec2 uv) {
    int width3D = inputTexelDimensions3D[0];
    int height3D = inputTexelDimensions3D[1];
    int length3D = inputTexelDimensions3D[2];
    int width2D = inputTexelDimensions2D[0];
    int height2D = inputTexelDimensions2D[1];
    float wStack = float(width2D)/float(width3D);
    float hStack = float(height2D)/float(height3D);
    float u = mod(uv[0]*wStack, 1.0);
    float v = mod(uv[1]*hStack, 1.0);
    float w = (floor(uv[1]*hStack)*wStack
               + floor(uv[0]*wStack) + 0.5)/float(length3D);
    return vec3(u, v, w);
}

void main() {
    vec2 uv;
    if (orientation == XY_SLICE) {
        float z = (float(slice) + 0.5)/float(inputTexelDimensions3D.z);
        uv = to2DTextureCoordinates(vec3(UV[0], UV[1], z));
    } else if (orientation == ZY_SLICE) {
        float x = (float(slice) + 0.5)/float(inputTexelDimensions3D.x);
        uv = to2DTextureCoordinates(vec3(x, UV[1], UV[0]));
    } else if (orientation == XZ_SLICE) {
        float y = (float(slice) + 0.5)/float(inputTexelDimensions3D.y);
        uv = to2DTextureCoordinates(vec3(UV[0], y, UV[1]));
    }
    fragColor = texture2D(tex, uv);
}