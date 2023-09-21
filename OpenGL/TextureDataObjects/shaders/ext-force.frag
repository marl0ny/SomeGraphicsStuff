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
uniform sampler2D positionsTex;
uniform sampler2D forcesTex;

void main() {
    vec2 r = texture2D(positionsTex, UV).xy;
    vec2 force1 = texture2D(forcesTex, UV).xy;
    float lW = 0.01, rW = 0.99;
    float uW = 0.99, dW = 0.01;
    vec2 leftWallForce
         = (r.x < lW)? vec2(-wall*(r.x - lW), 0.0): vec2(0.0, 0.0);
    vec2 rightWallForce
         = (r.x > rW)? vec2(-wall*(r.x - rW), 0.0): vec2(0.0, 0.0);
    vec2 ceilForce
         = (r.y > uW)? vec2(0.0, -wall*(r.y - uW)): vec2(0.0, 0.0);
    vec2 floorForce
         = (r.y < dW)? vec2(0.0, -wall*(r.y - dW)): vec2(0.0, 0.0);
    vec2 force2 = force1 + vec2(0.0, gravity)
                  + leftWallForce + rightWallForce + ceilForce + floorForce;
    fragColor = vec4(force2, force2);
}
