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

uniform sampler2D prevIterTex; // previous iteration for x
uniform sampler2D bTex; // b in b = A x

uniform float width;
uniform float height;
uniform float dx;
uniform float dy;


vec4 offdiagLaplacian2ndOrder(sampler2D tex) {
    vec4 u = texture(tex, UV + vec2(0.0, dy/height));
    vec4 d = texture(tex, UV - vec2(0.0, dy/height));
    vec4 l = texture(tex, UV - vec2(dx/width, 0.0));
    vec4 r = texture(tex, UV + vec2(dx/width, 0.0));
    return (u + d)/(dy*dy) + (l + r)/(dx*dx);

}

void main() {
    vec4 xPrev = texture2D(prevIterTex, UV);
    vec4 b = texture2D(bTex, UV);
    float diag = -2.0/(dy*dy) - 2.0/(dx*dx);
    fragColor = (b - offdiagLaplacian2ndOrder(prevIterTex))/diag;

}