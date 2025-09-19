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

uniform sampler2D coordTex;
uniform float circleRadius;
uniform vec2 dimensions2D;
uniform float scale;
uniform vec2 translate;

const float VERTEX_TYPE_IN_CENTER = 0.0;

void main() {
    UV = position.xy;
    float circleAngle = position[2];
    float vertexType = position[3];
    vec4 coord = texture2D(coordTex, UV);
    float texX = coord.x/dimensions2D[0];
    float texY = coord.y/dimensions2D[1];
    float c = cos(circleAngle), s = sin(circleAngle);
    vec2 texPos = vec2(texX, texY);
    if (vertexType != VERTEX_TYPE_IN_CENTER)
        texPos += circleRadius*vec2(c, s);
    gl_Position = vec4(scale*(2.0*texPos - vec2(1.0) - translate), 
                       0.0, 1.0);
}
