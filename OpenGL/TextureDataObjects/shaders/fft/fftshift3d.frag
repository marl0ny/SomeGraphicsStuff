#VERSION_NUMBER_PLACEHOLDER

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

uniform ivec3 texelDimensions3D;
uniform ivec2 texelDimensions2D;
uniform int orientation;

const int ORIENTATION_0 = 0;
const int ORIENTATION_1 = 1;
const int ORIENTATION_2 = 2;


vec2 to2DTextureCoordinates(vec3 position) {
    int width2D = texelDimensions2D[0];
    int width3D = texelDimensions3D[0];
    int length3D = texelDimensions3D[2];
    float xIndex = float(width3D)*mod(position.x, 1.0);
    float zIndex = float(length3D)*mod(position.z, 1.0);
    float uIndex = floor(zIndex)*float(width3D) + xIndex; 
    return vec2(uIndex/float(width2D), mod(position.y, 1.0));
}

vec3 to3DTextureCoordinates(vec2 uv) {
    int length3D = texelDimensions3D[2];
    float u = mod(uv[0]*float(length3D), 1.0);
    float v = uv[1];
    float w = (floor(uv[0]*float(length3D)) + 0.5)/float(length3D);
    return vec3(u, v, w);
}

void main() {
    vec3 uvw = to3DTextureCoordinates(UV);
    if (uvw[orientation] < 0.5) {
        uvw[orientation] += 0.5;
    } else {
        uvw[orientation] -= 0.5;
    }
    fragColor = texture2D(tex, to2DTextureCoordinates(uvw));
}
