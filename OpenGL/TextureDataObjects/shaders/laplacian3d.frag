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

uniform vec3 dr; // Spartial step sizes
uniform vec3 dimensions3D; // Dimensions of simulation

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


vec4 xLaplacian2ndOrder3Point(sampler2D tex) {
    vec3 xyz = to3DTextureCoordinates(UV).xyz;
    float x = xyz.x, y = xyz.y, z = xyz.z;
    float xf1 = x + dr[0]/dimensions3D[0];
    float xf2 = x + 2.0*dr[0]/dimensions3D[0];
    float xb1 = x - dr[0]/dimensions3D[0];
    float xb2 = x - 2.0*dr[0]/dimensions3D[0];
    vec4 r2 = texture2D(tex, to2DTextureCoordinates(vec3(xf2, y, z)));
    vec4 r1 = texture2D(tex, to2DTextureCoordinates(vec3(xf1, y, z)));
    vec4 c0 = texture2D(tex, UV);
    vec4 L1 = texture2D(tex, to2DTextureCoordinates(vec3(xb1, y, z)));
    vec4 L2 = texture2D(tex, to2DTextureCoordinates(vec3(xb2, y, z)));
    return (r1 - 2.0*c0 + L1)/(dr[0]*dr[0]);
}

vec4 yLaplacian2ndOrder3Point(sampler2D tex) {
    vec3 xyz = to3DTextureCoordinates(UV).xyz;
    float x = xyz.x, y = xyz.y, z = xyz.z;
    float yf1 = y + dr[1]/dimensions3D[1];
    float yf2 = y + 2.0*dr[1]/dimensions3D[1];
    float yb1 = y - dr[1]/dimensions3D[1];
    float yb2 = y - 2.0*dr[1]/dimensions3D[1];
    vec4 u2 = texture2D(tex, to2DTextureCoordinates(vec3(x, yf2, z)));
    vec4 u1 = texture2D(tex, to2DTextureCoordinates(vec3(x, yf1, z)));
    vec4 c0 = texture2D(tex, UV);
    vec4 d1 = texture2D(tex, to2DTextureCoordinates(vec3(x, yb1, z)));
    vec4 d2 = texture2D(tex, to2DTextureCoordinates(vec3(x, yb2, z)));
    return (u1 - 2.0*c0 + d1)/(dr[1]*dr[1]);
}

vec4 zLaplacian2ndOrder3Point(sampler2D tex) {
    vec3 xyz = to3DTextureCoordinates(UV).xyz;
    float x = xyz.x, y = xyz.y, z = xyz.z;
    float zf1 = z + dr[2]/dimensions3D[2];
    float zf2 = z + 2.0*dr[2]/dimensions3D[2];
    float zb1 = z - dr[2]/dimensions3D[2];
    float zb2 = z - 2.0*dr[2]/dimensions3D[2];
    vec4 f2 = texture2D(tex, to2DTextureCoordinates(vec3(x, y, zf2)));
    vec4 f1 = texture2D(tex, to2DTextureCoordinates(vec3(x, y, zf1)));
    vec4 c0 = texture2D(tex, UV);
    vec4 b1 = texture2D(tex, to2DTextureCoordinates(vec3(x, y, zb1)));
    vec4 b2 = texture2D(tex, to2DTextureCoordinates(vec3(x, y, zb2)));
    return (f1 - 2.0*c0 + b1)/(dr[2]*dr[2]);
}



vec4 xLaplacian4thOrder5Point(sampler2D tex) {
    vec3 xyz = to3DTextureCoordinates(UV).xyz;
    float x = xyz.x, y = xyz.y, z = xyz.z;
    float xf1 = x + dr[0]/dimensions3D[0];
    float xf2 = x + 2.0*dr[0]/dimensions3D[0];
    float xb1 = x - dr[0]/dimensions3D[0];
    float xb2 = x - 2.0*dr[0]/dimensions3D[0];
    vec4 r2 = texture2D(tex, to2DTextureCoordinates(vec3(xf2, y, z)));
    vec4 r1 = texture2D(tex, to2DTextureCoordinates(vec3(xf1, y, z)));
    vec4 c0 = texture2D(tex, UV);
    vec4 L1 = texture2D(tex, to2DTextureCoordinates(vec3(xb1, y, z)));
    vec4 L2 = texture2D(tex, to2DTextureCoordinates(vec3(xb2, y, z)));
    return (-r2/12.0 + 4.0*r1/3.0 - 5.0*c0/2.0
               + 4.0*L1/3.0 - L2/12.0)/(dr[0]*dr[0]);
}

vec4 yLaplacian4thOrder5Point(sampler2D tex) {
    vec3 xyz = to3DTextureCoordinates(UV).xyz;
    float x = xyz.x, y = xyz.y, z = xyz.z;
    float yf1 = y + dr[1]/dimensions3D[1];
    float yf2 = y + 2.0*dr[1]/dimensions3D[1];
    float yb1 = y - dr[1]/dimensions3D[1];
    float yb2 = y - 2.0*dr[1]/dimensions3D[1];
    vec4 u2 = texture2D(tex, to2DTextureCoordinates(vec3(x, yf2, z)));
    vec4 u1 = texture2D(tex, to2DTextureCoordinates(vec3(x, yf1, z)));
    vec4 c0 = texture2D(tex, UV);
    vec4 d1 = texture2D(tex, to2DTextureCoordinates(vec3(x, yb1, z)));
    vec4 d2 = texture2D(tex, to2DTextureCoordinates(vec3(x, yb2, z)));
    return (-u2/12.0 + 4.0*u1/3.0 - 5.0*c0/2.0
               + 4.0*d1/3.0 - d2/12.0)/(dr[1]*dr[1]);
}

vec4 zLaplacian4thOrder5Point(sampler2D tex) {
    vec3 xyz = to3DTextureCoordinates(UV).xyz;
    float x = xyz.x, y = xyz.y, z = xyz.z;
    float zf1 = z + dr[2]/dimensions3D[2];
    float zf2 = z + 2.0*dr[2]/dimensions3D[2];
    float zb1 = z - dr[2]/dimensions3D[2];
    float zb2 = z - 2.0*dr[2]/dimensions3D[2];
    vec4 f2 = texture2D(tex, to2DTextureCoordinates(vec3(x, y, zf2)));
    vec4 f1 = texture2D(tex, to2DTextureCoordinates(vec3(x, y, zf1)));
    vec4 c0 = texture2D(tex, UV);
    vec4 b1 = texture2D(tex, to2DTextureCoordinates(vec3(x, y, zb1)));
    vec4 b2 = texture2D(tex, to2DTextureCoordinates(vec3(x, y, zb2)));
    return (-f2/12.0 + 4.0*f1/3.0 - 5.0*c0/2.0
               + 4.0*b1/3.0 - b2/12.0)/(dr[2]*dr[2]);
}

void main() {
    vec4 d2Texdx2 = xLaplacian2ndOrder3Point(tex);
    vec4 d2Texdy2 = yLaplacian2ndOrder3Point(tex);
    vec4 d2Texdz2 = zLaplacian2ndOrder3Point(tex);
    /* vec4 d2Texdx2 = xLaplacian4thOrder5Point(tex);
    vec4 d2Texdy2 = yLaplacian4thOrder5Point(tex);
    vec4 d2Texdz2 = zLaplacian4thOrder5Point(tex);*/
    // fragColor = texture2D(tex, UV);
    fragColor = d2Texdx2 + d2Texdy2 + d2Texdz2;
}