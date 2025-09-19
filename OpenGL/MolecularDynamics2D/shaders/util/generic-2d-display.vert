#if __VERSION__ <= 120
attribute vec4 position;
varying vec2 UV;
#else
in vec4 position;
out vec2 UV;
#endif

#if (__VERSION__ >= 330) || (defined(GL_ES) && __VERSION__ >= 300)
#define texture2D texture
#else
#define texture texture2D
#endif

#if (__VERSION__ > 120) || defined(GL_ES)
precision highp float;
#endif

uniform float scale;
uniform vec2 translate;

void main() {
    UV = position.xy;
    gl_Position = vec4(scale*(
        2.0*position.xy - vec2(1.0) - translate), 0.0, 1.0);
}
