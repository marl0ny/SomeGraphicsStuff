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

vec2 getExtWallForce(vec2 r, 
                     float lW, float rW, float uW, float dW) {
    vec2 leftWallForce
         = (r.x < lW)? vec2(-wall*(r.x - lW), 0.0): vec2(0.0, 0.0);
    vec2 rightWallForce
         = (r.x > rW)? vec2(-wall*(r.x - rW), 0.0): vec2(0.0, 0.0);
    vec2 ceilForce
         = (r.y > uW)? vec2(0.0, -wall*(r.y - uW)): vec2(0.0, 0.0);
    vec2 floorForce
         = (r.y < dW)? vec2(0.0, -wall*(r.y - dW)): vec2(0.0, 0.0);
    return leftWallForce + rightWallForce + ceilForce + floorForce;
    
}

void main() {
    float index = (isRow)? UV[0]: UV[1];
    vec2 r0 = (isRow)?
        texture2D(positionsTex, vec2(index, 0.5)).xy:
        texture2D(positionsTex, vec2(0.5, index)).xy;
    vec2 intForce = vec2(0.0, 0.0);
    float intEnergy = 0.0;
    float sigma2 = sigma*sigma;
    float sigma6 = sigma2*sigma2*sigma2;
    float sigma12 = sigma6*sigma6;
    float extWallEnergy = getExtWallEnergy(r0, 0.01, 0.99, 0.99, 0.01);
    vec2 extWallForce = getExtWallForce(r0, 0.01, 0.99, 0.99, 0.01);
    float gravityEnergy = -gravity*r0.y; 
    vec2 gravityForce = vec2(0.0, gravity);
    for (float i = 0.5; i < nParticles; i++) {
        vec2 r1 = (isRow)?
            texture2D(positionsTex, vec2(i/nParticles, 0.5)).xy:
            texture2D(positionsTex, vec2(0.5, i/nParticles)).xy;
        vec2 r = r1 - r0;
        float r2 = r.x*r.x + r.y*r.y;
        if (abs(index - (i/nParticles)) > 1e-30
            // && r2 < (3.0*3.0)*sigma2
           ) {
            float r4 = r2*r2;
            float r6 = r4*r2;
            float r8 = r4*r4;
            float r12 = r8*r4;
            float r14 = r12*r2;
            float s1 = (4.0*epsilon)*(-12.0*sigma12/r14);
            float s2 = (4.0*epsilon)*(6.0*sigma6/r8);
            intEnergy += 2.0*epsilon*(sigma12/r12 - sigma6/r6);
            intForce += r*(s1 + s2);
        }
    }
    fragColor = vec4(intForce + extWallForce + gravityForce,
                     intEnergy + extWallEnergy + gravityEnergy, 0.0);
}
