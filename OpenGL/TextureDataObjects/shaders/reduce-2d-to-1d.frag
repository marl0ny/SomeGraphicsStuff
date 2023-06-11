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
uniform int sumOverIndex;
uniform int srcWidth;
uniform int srcHeight;
uniform bool dstIsRow;

void main() {
    vec4 sumVal = vec4(0.0, 0.0, 0.0, 0.0);
    if (sumOverIndex == 0) {
        for (int i = 0; i < srcWidth; i++) {
            float u = (float(i) + 0.5)/srcWidth;
            sumVal += (dstIsRow)?
                texture2D(tex, vec2(u, UV[0])):
                texture2D(tex, vec2(u, UV[1]));
        }
    } else {
        for (int i = 0; i < srcHeight; i++) {
            float v = (float(i) + 0.5)/srcHeight;
            sumVal += (dstIsRow)?
                texture2D(tex, vec2(UV[0], v)):
                texture2D(tex, vec2(UV[1], v));
        }
    }
    fragColor = sumVal;
}