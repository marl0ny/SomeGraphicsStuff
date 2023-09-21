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

uniform float gravity;
uniform float wall;
uniform float sigma;
uniform float epsilon;
uniform float nParticles;
uniform sampler2D positionsTex;
uniform bool isRow;

float getExtWallEnergy(vec2 r, 
                       float lW, float rW, float uW, float dW) {
    float leftWallE
         = (r.x < lW)? 0.5*wall*pow(r.x - lW, 2.0): 0.0;
    float rightWallE
         = (r.x > rW)? 0.5*wall*pow(r.x - rW, 2.0): 0.0;
    float ceilE
         = (r.y > uW)? 0.5*wall*pow(r.y - uW, 2.0): 0.0;
    float floorE
         = (r.y < dW)? 0.5*wall*pow(r.y - dW, 2.0): 0.0;
    return leftWallE + rightWallE + ceilE + floorE;
}

void main() {
    float index = (isRow)? UV[0]: UV[1];
    vec2 r0 = (isRow)?
        texture2D(positionsTex, vec2(index, 0.5)).xy:
        texture2D(positionsTex, vec2(0.5, index)).xy;
    float energy = 0.0;
    float sigma12 = pow(sigma, 12.0);
    float sigma6 = pow(sigma, 6.0);
    for (float i = 0.5; i < nParticles; i++) {
        vec2 r1 = (isRow)?
            texture2D(positionsTex, vec2(i/nParticles, 0.5)).xy:
            texture2D(positionsTex, vec2(0.5, i/nParticles)).xy;
        vec2 r = r1 - r0;
        float r2 = r.x*r.x + r.y*r.y;
        float r4 = r2*r2;
        float r6 = r4*r2;
        float r8 = r4*r4;
        float r12 = r8*r4;
        if (abs(index - (i/nParticles)) > 1e-30)
            energy += 2.0*epsilon*(sigma12/r12 - sigma6/r6);
    }
    float extWallE = getExtWallEnergy(r0, 0.01, 0.99, 0.99, 0.01);
    fragColor = vec4(energy - gravity*r0.y + extWallE);
}
