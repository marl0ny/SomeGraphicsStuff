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

uniform float sigma;
uniform float epsilon;
uniform float nParticles;
uniform sampler2D positionsTex;
uniform bool isRow;

void main() {
    float index = (isRow)? UV[0]: UV[1];
    vec2 r0 = (isRow)?
        texture2D(positionsTex, vec2(index, 0.5)).xy:
        texture2D(positionsTex, vec2(0.5, index)).xy;
    vec2 force = vec2(0.0, 0.0);
    float sigma12 = pow(sigma, 12.0);
    float sigma6 = pow(sigma, 6.0);
    for (float i = 0.5; i < nParticles; i++) {
        vec2 r1 = (isRow)?
            texture2D(positionsTex, vec2(i/nParticles, 0.5)).xy:
            texture2D(positionsTex, vec2(0.5, i/nParticles)).xy;
        vec2 r = r1 - r0;
        float r2 = r.x*r.x + r.y*r.y;
        float r4 = r2*r2;
        float r8 = r4*r4;
        float r14 = r8*r4*r2;
        float s1 = (4.0*epsilon)*(-12.0*sigma12/r14);
        float s2 = (4.0*epsilon)*(6.0*sigma6/r8);
        if (abs(index - (i/nParticles)) > 1e-30)
            force += r*(s1 + s2);
    }
    fragColor = vec4(force, force);
}
