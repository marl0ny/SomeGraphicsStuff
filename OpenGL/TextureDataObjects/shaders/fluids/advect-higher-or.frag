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

uniform float dx;
uniform float dy;
uniform float width;
uniform float height;
uniform float dt;

uniform sampler2D tex;
uniform sampler2D velocityTex;
uniform sampler2D advectForwardTex;
uniform sampler2D advectReverseTex;

float clamp4(float val, float c0, float c1, float c2, float c3) {
    float maxC = max(max(max(c0, c1), c2), c3);
    float minC = min(min(min(c0, c1), c2), c3);
    if (val > maxC)
        return maxC;
    else if (val < minC)
        return minC;
    return val;
}

void main() {

    vec4 original = texture2D(tex, UV);
    vec4 forward = texture2D(advectForwardTex, UV);
    vec4 reverse = texture2D(advectReverseTex, UV);
    vec4 updated = forward + 0.5*(original - reverse);

    vec2 vel = texture2D(velocityTex, UV).xy;
    vec2 texVel = vec2(vel.x/width, vel.y/height);
    float texelWidth = width/dx, texelHeight = height/dy;
    vec2 r = UV - dt*texVel;
    vec2 rUL = vec2((floor(r.x*texelWidth - 0.5) + 0.5)/texelWidth, 
                    (ceil(r.y*texelHeight - 0.5) + 0.5)/texelHeight);
    vec2 rUR = vec2((ceil(r.x*texelWidth - 0.5) + 0.5)/texelWidth, 
                    (ceil(r.y*texelHeight - 0.5) + 0.5)/texelHeight);
    vec2 rDL = vec2((floor(r.x*texelWidth - 0.5) + 0.5)/texelWidth, 
                    (floor(r.y*texelHeight - 0.5) + 0.5)/texelHeight);
    vec2 rDR = vec2((ceil(r.x*texelWidth - 0.5) + 0.5)/texelWidth, 
                    (floor(r.y*texelHeight - 0.5) + 0.5)/texelHeight);
    vec4 valC = updated;
    vec4 valUL = texture2D(tex, rUL);
    vec4 valUR = texture2D(tex, rUR);
    vec4 valDL = texture2D(tex, rDL);
    vec4 valDR = texture2D(tex, rDR);
    fragColor = vec4(clamp4(valC[0], valUL[0], valUR[0], valDL[0], valDR[0]),
                     clamp4(valC[1], valUL[1], valUR[1], valDL[1], valDR[1]),
                     clamp4(valC[2], valUL[2], valUR[2], valDL[2], valDR[2]),
                     clamp4(valC[3], valUL[3], valUR[3], valDL[3], valDR[3]));
}





