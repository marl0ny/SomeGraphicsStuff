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

uniform sampler2D tex;

uniform ivec3 texelDimensions3D;
uniform ivec2 texelDimensions2D;


vec2 to2DTextureCoordinates(vec3 position) {
    int width2D = texelDimensions2D[0];
    int width3D = texelDimensions3D[0];
    int length3D = texelDimensions3D[2];
    float xIndex = float(width3D)*mod(position.x, 1.0);
    float zIndex = float(length3D)*mod(position.z, 1.0);
    float uIndex = floor(zIndex)*float(width3D) + xIndex; 
    return vec2(uIndex/float(width2D), mod(position.y, 1.0));
}

vec3 to3DTextureCoordinates(vec2 uv) {
    int length3D = texelDimensions3D[2];
    float u = mod(uv[0]*float(length3D), 1.0);
    float v = uv[1];
    float w = (floor(uv[0]*float(length3D)) + 0.5)/float(length3D);
    return vec3(u, v, w);
}

vec3 revBitSort2(vec3 uvw) {
    vec3 uvw2 = vec3(0.0, 0.0, 0.0);
    int indexU = int(floor(uvw[0]*float(texelDimensions3D[0])));
    int indexV = int(floor(uvw[1]*float(texelDimensions3D[1])));
    int indexW = int(floor(uvw[2]*float(texelDimensions3D[2])));
    // u
    int rev = int(0), i = indexU;
    for (int asc = 1,
         des = texelDimensions3D[0]/2; des > 0; des /= 2, asc *= 2) {
        if (i/des > 0) {
            rev += asc;
            i -= des;
        }
    }
    uvw2[0] = (float(rev) + 0.5)/float(texelDimensions3D[0]);
    // v
    rev = 0, i = indexV;
    for (int asc = 1,
         des = texelDimensions3D[1]/2; des > 0; des /= 2, asc *= 2) {
        if (i/des > 0) {
            rev += asc;
            i -= des;
        }
    }
    uvw2[1] = (float(rev) + 0.5)/float(texelDimensions3D[1]);
    // w
    rev = 0, i = indexW;
    for (int asc = 1,
         des = texelDimensions3D[2]/2; des > 0; des /= 2, asc *= 2) {
        if (i/des > 0) {
            rev += asc;
            i -= des;
        }
    }
    uvw2[2] = (float(rev) + 0.5)/float(texelDimensions3D[2]);
    return uvw2;
}


void main() {
    vec3 uvw = to3DTextureCoordinates(UV);
    fragColor = texture2D(tex, to2DTextureCoordinates(revBitSort2(uvw)));
}