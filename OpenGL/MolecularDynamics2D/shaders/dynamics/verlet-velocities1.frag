/* Advance the velocities using the forces that were computed 
from the initial positions, where it is assumed that in the 
input positions-velocity texture, the positions for the next step 
have already been computed, but unsorted. 
Since the positions for the next step have yet to be sorted, 
a different shader in an additional pass is needed to complete 
the Verlet step for the velocities.

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
    vec2 v0 = texture2D(positionVelocitiesTex, UV).zw;
    vec2 forces = texture2D(forcesTex, UV).xy;
    vec2 vHalf = v0 + 0.5*forces*dt;
    // if (length(vHalf) > 100.0)
    //     vHalf = 100.0*normalize(vHalf);
    fragColor = vec4(r1, vHalf);
}