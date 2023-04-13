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
// uniform bool isVertical;


void main() {
    float u0 = UV[0], v0 = UV[1];
    float u = (u0 < 0.5)? u0 + 0.5: u0 - 0.5;
    float v = (v0 < 0.5)? v0 + 0.5: v0 - 0.5;
    fragColor = texture2D(tex, vec2(u, v));
    /*if (!isVertical)
        fragColor = (u < 0.5)?
            texture2D(tex, vec2(u + 0.5, v)): 
            texture2D(tex, vec2(u - 0.5, v));
    else
        fragColor = (v < 0.5)? 
            texture2D(tex, vec2(u, v + 0.5)): 
            texture2D(tex, vec2(u, v - 0.5));*/
}