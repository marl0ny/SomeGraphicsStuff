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

uniform float amplitude;
uniform float sigma;
uniform vec2 r0;
uniform vec2 v0;

void main() {
    float u = UV[0] - r0[0];
    float v = UV[1] - r0[1];
    float amp = amplitude*exp(-0.5*(u*u + v*v)/(sigma*sigma));
    fragColor = amp*vec4(v0, v0);
}