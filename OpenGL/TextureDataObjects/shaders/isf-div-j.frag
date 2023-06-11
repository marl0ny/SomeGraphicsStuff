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

#define complex vec2
#define complex2 vec4

uniform float dx;
uniform float dy;
uniform float width;
uniform float height;
uniform sampler2D texU;
uniform sampler2D texD;

const float PI = 3.141592653589793;


complex2 psiAt(vec2 uv) {
    return complex2(texture2D(texU, uv).xy,
                    texture2D(texD, uv).xy);
}

complex conj(complex psi) {
    return complex(psi.x, -psi.y);
}

complex mul(complex w, complex z) {
    return complex(w.x*z.x - w.y*z.y, w.x*z.y + w.y*z.x);
}

complex innerProd(complex2 a, complex2 b) {
    return mul(conj(a.xy), b.xy) + mul(conj(a.zw), b.zw);
}

float arg(vec2 v) {
    return (v.y > 0)? atan(v.y, v.x): 2.0*PI + atan(v.y, v.x);
}

float dJdr(vec2 b, vec2 c, vec2 f, float d) {
    float angle1 = arg(innerProd(psiAt(b), psiAt(c)));
    float angle2 = arg(innerProd(psiAt(c), psiAt(f)));
    // return (angle2 + angle1)/(2.0*d*d);
    float diff1 = angle2 - angle1;
    // return diff1/(2.0*d*d);
    float diff2 = (angle2 > angle1)?
          (2.0*PI - angle2 + angle1): -(2.0*PI - angle1 + angle2);
    return ((abs(diff2) > abs(diff1))? diff1: diff2)/(2.0*d*d);
}


void main() {
    float dJdx = dJdr(UV - vec2(dx/width, 0.0), UV,
                      UV + vec2(dx/width, 0.0), dx);
    float dJdy = dJdr(UV - vec2(0.0, dy/height), UV,
                      UV + vec2(0.0, dy/height), dy);
    fragColor = vec4(dJdx + dJdy);          
}