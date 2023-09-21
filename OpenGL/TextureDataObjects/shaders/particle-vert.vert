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
attribute vec2 uvIndex;
#else
in vec2 uvIndex;
#endif

uniform sampler2D tex;

void main() {
    vec4 posVel = texture2D(tex, uvIndex);
    gl_Position = vec4(2.0*posVel.xy - 1.0, 0.0, 1.0);
}
