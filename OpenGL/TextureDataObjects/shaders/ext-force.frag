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
