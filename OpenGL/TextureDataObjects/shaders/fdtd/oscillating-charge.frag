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

uniform float t;

uniform vec3 oscillationAmplitude;
uniform float phi;
uniform float omega;

uniform float chargeAmplitude;
uniform vec3 sigma;

uniform ivec2 texelDimensions2D;
uniform ivec3 texelDimensions3D;
uniform vec3 dimensions3D; // Dimensions of simulation


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

void main() {
    vec3 uvw = to3DTextureCoordinates(UV);
    vec3 r = oscillationAmplitude*cos(omega*t - phi);
    vec3 v = -oscillationAmplitude*omega*sin(omega*t - phi);
    float r0 = (uvw[0] - 0.5) - r[0]/dimensions3D[0];
    float r1 = (uvw[1] - 0.5) - r[1]/dimensions3D[1];
    float r2 = (uvw[2] - 0.5) - r[2]/dimensions3D[2];
    float s0 = sigma[0]/dimensions3D[0];
    float s1 = sigma[1]/dimensions3D[1];
    float s2 = sigma[2]/dimensions3D[2];
    float rho = chargeAmplitude*exp(-0.5*(r0*r0/(s0*s0)
                                          + r1*r1/(s1*s1) + r2*r2/(s2*s2)));
    fragColor = vec4(v*rho, rho);
}

