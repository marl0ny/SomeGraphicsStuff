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

/* This assumes that the texture's height is only
one pixel.
*/


uniform sampler2D tex;
uniform ivec2 texelDimensions2D;
uniform ivec3 texelDimensions3D;

uniform vec3 dr;
uniform vec3 dimensions3D;


void main() {

}