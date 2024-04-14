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

#define quaternion vec4

uniform vec4 rotation;
uniform sampler2D gradientTex;
uniform sampler2D densityTex;

uniform ivec3 texelDimensions3D;
uniform ivec2 texelDimensions2D;


void main() {
    vec3 grad;
    vec4 density;
    for (int i = 0; i < 64; i++) {
        grad = grad + texture2D(gradientTex, UV).xyz;
        density = density + texture2D(densityTex, UV);
    }
    gl_FragColor = density + vec4(grad, 0.0);
}