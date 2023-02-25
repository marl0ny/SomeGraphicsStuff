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

uniform sampler2D posVelTex;
uniform vec2 verticalBoundaries;
uniform vec2 horizontalBoundaries;


/*
Enforce periodic boundary conditions for the
positions of the particles.
*/
void main() {
    float xi = horizontalBoundaries[0];
    float xf = horizontalBoundaries[1];
    float yi = verticalBoundaries[0]; 
    float yf = verticalBoundaries[1];
    float xRange = xf - xi;
    float yRange = yf - yi;
    vec4 posVel = texture2D(posVelTex, UV);
    vec2 pos = posVel.xy - vec2(xi, yi);
    vec2 vel = posVel.zw;
    if (pos.x < 0.0) pos.x += xRange;
    if (pos.y < 0.0) pos.y += yRange;
    pos.x = mod(pos.x, xRange) + xi;
    pos.y = mod(pos.y, yRange) + yi;
    fragColor = vec4(pos, vel);
}