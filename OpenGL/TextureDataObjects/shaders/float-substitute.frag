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
uniform float oldVal;
uniform float newVal;


void main() {
    vec4 originalTexVal = texture2D(tex, UV);
    if (originalTexVal.r == oldVal)
        fragColor = vec4(newVal, newVal, newVal, newVal);
    else
        fragColor = originalTexVal;

}