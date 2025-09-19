/* Final shader in the Verlet velocity step implementation.
It is assumed the positions have already been updated, that
the velocities have been advanced using the forces computed
from the initial positions only, and that the input 
positions-velocity texture has been properly sorted. 
The shader takes as input the texture of forces
that has been computed from the sorted, updated positions,
and adds this to the velocity to advance it fully to the next step.

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

void main() {
    vec2 r1 = texture2D(positionVelocitiesTex, UV).xy;
    vec2 vHalf = texture2D(positionVelocitiesTex, UV).zw;
    vec2 forces = texture2D(forcesTex, UV).xy;
    vec2 v1 = vHalf + 0.5*forces*dt;
    // if (length(v1) > 100.0)
    //     v1 = 100.0*normalize(v1);
    fragColor = vec4(r1, v1);
}