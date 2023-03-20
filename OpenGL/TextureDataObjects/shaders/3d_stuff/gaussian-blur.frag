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

uniform sampler2D tex;
uniform bool isVertical;
uniform int width;
uniform int height;


void main() {
    vec4 val = vec4(0.0, 0.0, 0.0, 0.0);
    for (int i = -15; i < 15; i++) {
        if (isVertical) {
            float s = 0.01;
            float v = (float(i) + 0.5)/float(height);
            float gaussVal = exp(-0.5*pow(v/s, 2.0));
            vec2 step = vec2(0.0, float(i)/float(height));
            val += gaussVal*texture2D(tex, UV + step);
        } else {
            float s = 0.01;
            float u = (float(i) + 0.5)/float(width);
            float gaussVal = exp(-0.5*pow(u/s, 2.0));
            vec2 step = vec2(float(i)/float(width), 0.0);
            val += gaussVal*texture2D(tex, UV + step);
        }
    }
    fragColor = vec4(val.xyz, 1.0);
}