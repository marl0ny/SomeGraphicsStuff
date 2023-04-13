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

uniform float dx;
uniform float dy;
uniform float width;
uniform float height;
uniform int orderOfAccuracy;

uniform int boundaryType;
#define NONE_SPECIFIED 0
#define DIRICHLET 1

uniform sampler2D tex;

vec4 laplacian2ndOrder4Point(sampler2D tex) {
    vec4 u = texture(tex, UV + vec2(0.0, dy/height));
    vec4 d = texture(tex, UV - vec2(0.0, dy/height));
    vec4 l = texture(tex, UV - vec2(dx/width, 0.0));
    vec4 r = texture(tex, UV + vec2(dx/width, 0.0));
    vec4 c = texture(tex, UV);
    /*if (boundaryType == DIRICHLET) {
        vec4 yIn = (u + d - 2.0*c)/(dy*dy);
        vec4 xIn = (l + r - 2.0*c)/(dx*dx);
        vec4 yUB = (d - 2.0*c)/(dy*dy); // y laplacian at upper boundary
        vec4 yDB = (u - 2.0*c)/(dy*dy); // y laplacian at down boundary
        vec4 xLB = (r - 2.0*c)/(dx*dx); // x laplacian at left most boundary
        vec4 xRB = (l - 2.0*c)/(dx*dx); // x laplacian at right most boundary
        vec4 in = xIn + yIn;
        vec4 lb = xLB + yIn; // left boundary (no corner)
        vec4 rb = xRB + yIn; // right
        vec4 ub = xIn + yUB; // up
        vec4 db = xIn + yDB; // down
        vec4 rub = xRB + yUB; // upper right corner
        vec4 lub = xLB + yUB; // upper left
        vec4 ldb = xLB + yDB; // lower left
        vec4 rdb = xRB + yDB; // lower right
        if (UV.x < dx && UV.y > dy && UV.y < (1.0 - dy))
            return lb;
        else if (UV.x > (1.0 - dx) && )
    }*/
    return (u + d - 2.0*c)/(dy*dy) + (l + r - 2.0*c)/(dx*dx);
}

vec4 laplacian4thOrder9Point(sampler2D tex) {
    vec4 u2 = texture(tex, UV + 2.0*vec2(0.0, dy/height));
    vec4 u1 = texture(tex, UV +     vec2(0.0, dy/height));
    vec4 d1 = texture(tex, UV -     vec2(0.0, dy/height));
    vec4 d2 = texture(tex, UV - 2.0*vec2(0.0, dy/height));
    vec4 L2 = texture(tex, UV - 2.0*vec2(dx/width, 0.0));
    vec4 L1 = texture(tex, UV -     vec2(dx/width, 0.0));
    vec4 r1 = texture(tex, UV +     vec2(dx/width, 0.0));
    vec4 r2 = texture(tex, UV + 2.0*vec2(dx/width, 0.0));
    vec4 c0 = texture(tex, UV);
    float dy2 = dy*dy, dx2 = dx*dx;
    return (-u2/12.0 + 4.0*u1/3.0 - 5.0*c0/2.0 + 4.0*d1/3.0 - d2/12.0)/dy2
            + (-L2/12.0 + 4.0*L1/3.0 - 5.0*c0/2.0 + 4.0*r1/3.0 - r2/12.0)/dx2;
}

vec4 laplacian6thOrder13Point(sampler2D tex) {
    vec4 u3 = texture(tex, UV + 3.0*vec2(0.0, dy/height));
    vec4 u2 = texture(tex, UV + 2.0*vec2(0.0, dy/height));
    vec4 u1 = texture(tex, UV +     vec2(0.0, dy/height));
    vec4 d1 = texture(tex, UV -     vec2(0.0, dy/height));
    vec4 d2 = texture(tex, UV - 2.0*vec2(0.0, dy/height));
    vec4 d3 = texture(tex, UV - 3.0*vec2(0.0, dy/height));
    vec4 L3 = texture(tex, UV - 3.0*vec2(dx/width, 0.0));
    vec4 L2 = texture(tex, UV - 2.0*vec2(dx/width, 0.0));
    vec4 L1 = texture(tex, UV -     vec2(dx/width, 0.0));
    vec4 r1 = texture(tex, UV +     vec2(dx/width, 0.0));
    vec4 r2 = texture(tex, UV + 2.0*vec2(dx/width, 0.0));
    vec4 r3 = texture(tex, UV + 3.0*vec2(dx/width, 0.0));
    vec4 c0 = texture(tex, UV);
    float dy2 = dy*dy, dx2 = dx*dx;
    vec4 d2Tdx2 = ((r3 + L3)/90.0 - 3.0*(r2 + L2)/20.0
                    + 3.0*(r1 + L1)/2.0 - 49.0*c0/18.0)/dx2;
    vec4 d2Tdy2 = ((u3 + d3)/90.0 - 3.0*(u2 + d2)/20.0
                    + 3.0*(u1 + d1)/2.0 - 49.0*c0/18.0)/dy2;
    return d2Tdx2 + d2Tdy2;
}

vec4 laplacian8thOrder17Point(sampler2D tex) {
    vec4 u4 = texture(tex, UV + 4.0*vec2(0.0, dy/height));
    vec4 u3 = texture(tex, UV + 3.0*vec2(0.0, dy/height));
    vec4 u2 = texture(tex, UV + 2.0*vec2(0.0, dy/height));
    vec4 u1 = texture(tex, UV +     vec2(0.0, dy/height));
    vec4 d1 = texture(tex, UV -     vec2(0.0, dy/height));
    vec4 d2 = texture(tex, UV - 2.0*vec2(0.0, dy/height));
    vec4 d3 = texture(tex, UV - 3.0*vec2(0.0, dy/height));
    vec4 d4 = texture(tex, UV - 4.0*vec2(0.0, dy/height));
    vec4 L4 = texture(tex, UV - 4.0*vec2(dx/width, 0.0));
    vec4 L3 = texture(tex, UV - 3.0*vec2(dx/width, 0.0));
    vec4 L2 = texture(tex, UV - 2.0*vec2(dx/width, 0.0));
    vec4 L1 = texture(tex, UV -     vec2(dx/width, 0.0));
    vec4 r1 = texture(tex, UV +     vec2(dx/width, 0.0));
    vec4 r2 = texture(tex, UV + 2.0*vec2(dx/width, 0.0));
    vec4 r3 = texture(tex, UV + 3.0*vec2(dx/width, 0.0));
    vec4 r4 = texture(tex, UV + 4.0*vec2(dx/width, 0.0));
    vec4 c0 = texture(tex, UV);
    float dy2 = dy*dy, dx2 = dx*dx;
    vec4 d2Tdx2 = (-(r4 + L4)/560.0 + 8*(r3 + L3)/315.0 - (r2 + L2)/5.0
                    + 8.0*(r1 + L1)/5.0 - 205.0*c0/72.0)/dx2;
    vec4 d2Tdy2 = (-(u4 + d4)/560.0 + 8*(u3 + d3)/315.0 - (u2 + d2)/5.0
                    + 8.0*(u1 + d1)/5.0 - 205.0*c0/72.0)/dy2;
    return d2Tdx2 + d2Tdy2;
}

void main() {
    if (orderOfAccuracy >= 8)
        fragColor = laplacian8thOrder17Point(tex);
    else if (orderOfAccuracy >= 6)
        fragColor = laplacian6thOrder13Point(tex);
    else if (orderOfAccuracy >= 4) 
        fragColor = laplacian4thOrder9Point(tex);
    else
        fragColor = laplacian2ndOrder4Point(tex);
}