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