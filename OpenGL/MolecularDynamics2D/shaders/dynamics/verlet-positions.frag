/* Verlet intergration scheme, where only the positions are advanced
in time. 

References:

D. Schroeder. "Interactive Molecular Dynamics."
Available: https://physics.weber.edu/schroeder/md/.

D. Schroeder, "Interactive Molecular Dynamics,"
American Journal of Physics, 83, 3, 210 - 218. 2015.
http://dx.doi.org/10.1119/1.4901185

*/
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

uniform float dt;
uniform sampler2D positionVelocitiesTex;
uniform sampler2D forcesTex;

uniform bool enforcePeriodicity;
uniform vec2 simDimensions2D;

void main() {
    vec2 r0 = texture2D(positionVelocitiesTex, UV).xy;
    vec2 v0 = texture2D(positionVelocitiesTex, UV).zw;
    vec2 forces = texture2D(forcesTex, UV).xy;
    vec2 r1 = r0 + v0*dt + (0.5*(dt*(dt*forces)));
    if (enforcePeriodicity) {
        r1 = mod(r1, simDimensions2D);
        if (r1.x < 0.0)
            r1.x = simDimensions2D.x + r1.x;
        if (r1.y < 0.0)
            r1.y = simDimensions2D.y + r1.y;
    }
    fragColor = vec4(r1, v0);
}