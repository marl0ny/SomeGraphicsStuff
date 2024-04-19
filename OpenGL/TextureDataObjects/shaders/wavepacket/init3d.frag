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

uniform float amplitude0;
uniform float amplitude1;
uniform vec3 r0;
uniform vec3 r1;
uniform vec3 sigma0;
uniform vec3 sigma1;
uniform vec3 wavenumber0;
uniform vec3 wavenumber1;


const float PI = 3.141592653589793;

vec2 to2DTextureCoordinates(vec3 position) {
    int maxTexDimension = 8192;
    float wStack = 0.0;
    float hStack = 0.0;
    int width2D = texelDimensions2D[0];
    int width3D = texelDimensions3D[0];
    int length3D = texelDimensions3D[2];
    float xIndex = float(width3D)*mod(position.x, 1.0);
    float zIndex = float(length3D)*mod(position.z, 1.0);
    float uIndex = floor(zIndex)*float(width3D) + xIndex; 
    return vec2(uIndex/float(width2D), position.y);
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
    float gaussian0 = exp(-0.5*pow((uvw[0] - r0[0])/sigma0[0], 2.0)
                          -0.5*pow((uvw[1] - r0[1])/sigma0[1], 2.0)
                          -0.5*pow((uvw[2] - r0[2])/sigma0[2], 2.0));
    float gaussian1 = exp(-0.5*pow((uvw[0] - r1[0])/sigma1[0], 2.0)
                          -0.5*pow((uvw[1] - r1[1])/sigma1[1], 2.0)
                          -0.5*pow((uvw[2] - r1[2])/sigma1[2], 2.0));
    vec2 phase0 = vec2(cos(2.0*PI*dot(wavenumber0, uvw)),
                       sin(2.0*PI*dot(wavenumber0, uvw)));
    vec2 phase1 = vec2(cos(2.0*PI*dot(wavenumber1, uvw)),
                       sin(2.0*PI*dot(wavenumber1, uvw)));
    // float bVal = texture2D(boundaryMaskTex, to2DTextureCoordinates(uvw))[0];

    int width2D = texelDimensions2D[0];
    int height2D = texelDimensions2D[1];
    int width3D = texelDimensions3D[0];
    int height3D = texelDimensions3D[1];
    int length3D = texelDimensions3D[2];

    // vec3 uvw_ = to3DTextureCoordinates(UV);
    // vec2 uv2 = to2DTextureCoordinates(uvw_);
    // fragColor = vec4(uv2[1], uv2[1], 0.0, 0.0);
    
    fragColor = vec4(amplitude0*gaussian0*phase0,
                     amplitude1*gaussian1*phase1);
}

