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

// Dimensions
uniform vec3 dimensions;
uniform float t;
uniform ivec2 texelDimensions2D;
uniform ivec3 texelDimensions3D;

uniform bool shiftOriginToCenter;

const float pi = 3.141592653589793;

#define ADDITIONAL_UNIFORMS
ADDITIONAL_UNIFORMS


vec3 to3DTextureCoordinates(vec2 uv) {
    int width3D = texelDimensions3D[0];
    int height3D = texelDimensions3D[1];
    int length3D = texelDimensions3D[2];
    int width2D = texelDimensions2D[0];
    int height2D = texelDimensions2D[1];
    float wStack = float(width2D)/float(width3D);
    float hStack = float(height2D)/float(height3D);
    float u = mod(uv[0]*wStack, 1.0);
    float v = mod(uv[1]*hStack, 1.0);
    float w = (floor(uv[1]*hStack)*wStack
               + floor(uv[0]*wStack) + 0.5)/float(length3D);
    return vec3(u, v, w);
}

int numberOfDimensions() {
    if (texelDimensions3D[0] == 0 ||
        texelDimensions3D[1] == 0 ||
        texelDimensions3D[2] == 0) {
        return 2;
    } else {
        return 3;
    }
}

vec3 getPosition() {
    if (numberOfDimensions() == 2)
        return vec3(dimensions[0]*UV[0], dimensions[1]*UV[1], 0.0);
    else // Use 3D
        return dimensions*to3DTextureCoordinates(UV);
}

#define AX_PLACEHOLDER 0.0
#define AY_PLACEHOLDER 0.0
#define AZ_PLACEHOLDER 0.0
#define AT_PLACEHOLDER 0.0

void main() {
    vec3 uvw = to3DTextureCoordinates(UV);
    float u = uvw[0];
    float v = uvw[1];
    float w = uvw[2];
    vec3 r = getPosition();
    float x = r.x;
    float y = r.y;
    float z = r.z;
    if (shiftOriginToCenter) {
        x -= dimensions[0]/2.0;
        y -= dimensions[1]/2.0;
        z -= (numberOfDimensions() == 2)? 0.0: dimensions[2]/2.0;
    }
    float ax = AX_PLACEHOLDER;
    float ay = AY_PLACEHOLDER;
    float az = AZ_PLACEHOLDER;
    float at = AT_PLACEHOLDER;
    fragColor = vec4(ax, ay, az, at);
}
