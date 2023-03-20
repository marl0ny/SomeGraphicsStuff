#VERSION_NUMBER_PLACEHOLDER

precision highp float;

#if __VERSION__ >= 300
in vec3 POSITION;
out vec4 fragColor;
#define texture2D texture
#else
#define fragColor gl_FragColor
varying highp vec3 POSITION;
#endif

uniform sampler2D tex;

uniform ivec3 texelDimensions3D;
uniform ivec2 texelDimensions2D;

vec2 to2DTextureCoordinates(vec3 position) {
    int width2D = texelDimensions2D[0];
    int height2D = texelDimensions2D[1];
    int width3D = texelDimensions3D[0];
    int height3D = texelDimensions3D[1];
    int length3D = texelDimensions3D[2];
    float wStack = float(width2D)/float(width3D);
    float hStack = float(height2D)/float(height3D);
    float u = position.x;
    float v = position.y;
    float w = position.z;
    float wRatio = 1.0/wStack;
    float hRatio = 1.0/hStack;
    float wIndex = w*float(length3D) - 0.5;
    vec2 wPosition = vec2(mod(wIndex, wStack)/wStack,
                          floor(wIndex/wStack)/hStack);
    return wPosition + vec2(u*wRatio, v*hRatio);
}

// bilinear interpolation
vec4 blI(vec2 r, float x0, float y0, float x1, float y1,
         vec4 w00, vec4 w10, vec4 w01, vec4 w11) {
    float dx = x1 - x0, dy = y1 - y0;
    float ax = (dx == 0.0)? 0.0: (r.x - x0)/dx;
    float ay = (dy == 0.0)? 0.0: (r.y - y0)/dy;
    return mix(mix(w00, w10, ax), mix(w01, w11, ax), ay);
}

/*
Currently this assumes that the texture being sampled from
is smaller for all dimensions than the texture being
rendered to.
*/
vec4 sample2DTextureAs3D(sampler2D tex, vec3 position) {
    vec3 r = position;
    float width3D = float(texelDimensions3D[0]);
    float height3D = float(texelDimensions3D[1]);
    float length3D = float(texelDimensions3D[2]);
    float x0 = (floor(r.x*width3D - 0.5) + 0.5)/width3D;
    float y0 = (floor(r.y*height3D - 0.5) + 0.5)/height3D;
    float z0 = (floor(r.z*length3D - 0.5) + 0.5)/length3D;
    float x1 = (ceil(r.x*width3D - 0.5) + 0.5)/width3D;
    float y1 = (ceil(r.y*height3D - 0.5) + 0.5)/height3D;
    float z1 = (ceil(r.z*length3D - 0.5) + 0.5)/length3D;
    vec3 r000 = vec3(x0, y0, z0);
    vec3 r100 = vec3(x1, y0, z0);
    vec3 r010 = vec3(x0, y1, z0);
    vec3 r001 = vec3(x0, y0, z1);
    vec3 r110 = vec3(x1, y1, z0);
    vec3 r101 = vec3(x1, y0, z1);
    vec3 r011 = vec3(x0, y1, z1);
    vec3 r111 = vec3(x1, y1, z1);
    vec4 f000 = texture2D(tex, to2DTextureCoordinates(r000));
    vec4 f100 = texture2D(tex, to2DTextureCoordinates(r100));
    vec4 f010 = texture2D(tex, to2DTextureCoordinates(r010));
    vec4 f001 = texture2D(tex, to2DTextureCoordinates(r001));
    vec4 f110 = texture2D(tex, to2DTextureCoordinates(r110));
    vec4 f101 = texture2D(tex, to2DTextureCoordinates(r101));
    vec4 f011 = texture2D(tex, to2DTextureCoordinates(r011));
    vec4 f111 = texture2D(tex, to2DTextureCoordinates(r111));
    vec4 f0 = blI(r.xy, x0, y0, x1, y1, f000, f100, f010, f110);
    vec4 f1 = blI(r.xy, x0, y0, x1, y1, f001, f101, f011, f111);
    // Originally I made a mistake with the interpolation
    // where I neglected to consider the edge case of sampling a point at
    // at z0 (or x0 or y0) which resulted in a zero denominator for
    // some calculations. This created black spots in the final render.
    float dz = z1 - z0;
    return mix(f0, f1, (dz == 0.0)? 0.0: (r.z - z0)/dz);
}


void main() {
    vec3 r = POSITION;
    // This check needs to be done to avoid a repeating effect
    // caused by sampling beyond the initial boundary.
    if (r.x < 0.0 || r.x >= 1.0 ||
        r.y < 0.0 || r.y >= 1.0 ||
        r.z < 0.0 || r.z >= 1.0) discard;
    fragColor = sample2DTextureAs3D(tex, r);
}
