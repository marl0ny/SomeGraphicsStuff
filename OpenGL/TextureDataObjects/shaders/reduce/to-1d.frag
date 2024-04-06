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

uniform sampler2D tex;
uniform int sumOverIndex;
uniform int srcWidth;
uniform int srcHeight;
uniform bool dstIsRow;

void main() {
    vec4 sumVal = vec4(0.0, 0.0, 0.0, 0.0);
    if (sumOverIndex == 0) {
        for (int i = 0; i < srcWidth; i++) {
            float u = (float(i) + 0.5)/float(srcWidth);
            sumVal += (dstIsRow)?
                texture2D(tex, vec2(u, UV[0])):
                texture2D(tex, vec2(u, UV[1]));
        }
    } else {
        for (int i = 0; i < srcHeight; i++) {
            float v = (float(i) + 0.5)/float(srcHeight);
            sumVal += (dstIsRow)?
                texture2D(tex, vec2(UV[0], v)):
                texture2D(tex, vec2(UV[1], v));
        }
    }
    fragColor = sumVal;
}