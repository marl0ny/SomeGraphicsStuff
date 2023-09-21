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