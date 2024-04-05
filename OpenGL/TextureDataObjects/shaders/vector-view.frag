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
varying vec4 COLOUR;
#define fragColor gl_FragColor
#else
in vec4 COLOUR;
out vec4 fragColor;
#endif


void main() {
    // if (length(COLOUR.rgb) < 0.01) discard;
    if (COLOUR.a <= 0.01) discard;
    fragColor = COLOUR;
}
