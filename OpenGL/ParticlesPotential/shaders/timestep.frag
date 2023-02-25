#VERSION_NUMBER_PLACEHOLDER

precision highp float;

#if __VERSION__ >= 300
in vec2 UV;
// in vec2 FORCE;
out vec4 fragColor;
#define texture2D texture
#else
#define fragColor gl_FragColor
varying highp vec2 UV;
// varying highp vec2 FORCE;
#endif

uniform float dt;
uniform float m;
uniform bool gradIsComplex;
uniform sampler2D texGradPotential;
uniform sampler2D texParticles0;
uniform sampler2D texParticles1;

void main() {
    vec4 posVel0 = texture2D(texParticles0, UV);
    vec4 posVel1 = texture2D(texParticles1, UV);
    vec4 vel = vec4(posVel1[2], posVel1[3], 0.0, 0.0);
    vec4 gradPotential = texture2D(texGradPotential, UV);
    vec2 force2 = (gradIsComplex)? gradPotential.xz: gradPotential.xy;
    vec4 force = vec4(0.0, 0.0, force2);
    fragColor = posVel0 + dt*vel + dt*force/m;
}