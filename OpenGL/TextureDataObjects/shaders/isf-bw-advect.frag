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

uniform float width;
uniform float height;
uniform float dt;

uniform sampler2D velocityTex;
uniform sampler2D densityTex;

void main() {
    vec2 vel = texture2D(velocityTex, UV).xy;
    vec2 texVel = vec2(vel.x/width, vel.y/height);
    fragColor = texture2D(densityTex, UV - dt*texVel);
}