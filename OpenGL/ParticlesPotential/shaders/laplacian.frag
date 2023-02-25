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
uniform float dx;
uniform float dy;

uniform int orderOfAccuracy;
uniform sampler2D tex;


vec4 laplacian2ndOrder4Point(sampler2D tex) {
    vec4 u = texture2D(tex, UV + vec2(0.0, dy/height));
    vec4 d = texture2D(tex, UV - vec2(0.0, dy/height));
    vec4 l = texture2D(tex, UV - vec2(dx/width, 0.0));
    vec4 r = texture2D(tex, UV + vec2(dx/width, 0.0));
    vec4 c = texture2D(tex, UV);
    return (u + d - 2.0*c)/(dy*dy) + (l + r - 2.0*c)/(dx*dx);
}

vec4 laplacian4thOrder9Point(sampler2D tex) {
    vec4 u2 = texture2D(tex, UV + 2.0*vec2(0.0, dy/height));
    vec4 u1 = texture2D(tex, UV +     vec2(0.0, dy/height));
    vec4 d1 = texture2D(tex, UV -     vec2(0.0, dy/height));
    vec4 d2 = texture2D(tex, UV - 2.0*vec2(0.0, dy/height));
    vec4 L2 = texture2D(tex, UV - 2.0*vec2(dx/width, 0.0));
    vec4 L1 = texture2D(tex, UV -     vec2(dx/width, 0.0));
    vec4 r1 = texture2D(tex, UV +     vec2(dx/width, 0.0));
    vec4 r2 = texture2D(tex, UV + 2.0*vec2(dx/width, 0.0));
    vec4 c0 = texture2D(tex, UV);
    float dy2 = dy*dy, dx2 = dx*dx;
    return (-u2/12.0 + 4.0*u1/3.0 - 5.0*c0/2.0 + 4.0*d1/3.0 - d2/12.0)/dy2
            + (-L2/12.0 + 4.0*L1/3.0 - 5.0*c0/2.0 + 4.0*r1/3.0 - r2/12.0)/dx2;
}

void main() {
    if (orderOfAccuracy <= 2) {
        fragColor = laplacian2ndOrder4Point(tex);
    } else {
        fragColor = laplacian4thOrder9Point(tex);
    }
}
