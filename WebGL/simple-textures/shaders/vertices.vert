precision mediump float;
attribute vec4 pos;
attribute vec2 texture;
attribute vec3 normal;
attribute vec4 color;
varying mediump vec2 vTexture;
varying mediump vec4 vColor;
varying mediump vec4 vPos;
varying mediump vec4 vNorm;
uniform mat4 modelView;
uniform mat4 rotation;
uniform mat4 proj;
uniform float scale;
uniform vec4 rotationQuaternion;


struct ZAxisAlignedFrustum {
    float nearPlaneWidth;
    float nearPlaneHeight;
    float zNear;
    float zFar;
};


vec4 quaternionMultiply(vec4 q1, vec4 q2) {
    return vec4(
        q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y, 
        q1.w*q2.y + q1.y*q2.w + q1.z*q2.x - q1.x*q2.z, 
        q1.w*q2.z + q1.z*q2.w + q1.x*q2.y - q1.y*q2.x,
        q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z
    );
}


vec4 rotate(vec4 x, vec4 r) {
    vec4 xr = quaternionMultiply(x, r);
    r.x = -r.x;
    r.y = -r.y;
    r.z = -r.z;
    vec4 x2 = quaternionMultiply(r, xr);
    x2.w = 1.0;
    return x2; 
}

vec4 quaternionNormInvert(vec4 q) {
    return vec4(-q.x, -q.y, -q.z, q.w);
}

vec4 projectZ(vec4 p, float zCameraPosition, 
              ZAxisAlignedFrustum f) {
    float zNear = f.zNear - zCameraPosition;
    float zFar = f.zFar - zCameraPosition;
    float z = p.z - zCameraPosition;
    return vec4(
        zNear*p.x/(f.nearPlaneWidth/2.0),
        zNear*p.y/(f.nearPlaneHeight/2.0),
        z*(zNear + zFar)/(zFar - zNear)
        -2.0*zNear*zFar/(zFar - zNear),
        z
    );
}


vec4 changeColor(vec4 color, vec4 position, vec3 lightRay) {
    vec3 position3 = vec3(position.x, position.y, position.z);
    vec3 normalLightRay = normalize(lightRay);
    vec3 normalPosition = normalize(position3);
    vec4 newColor;
    for (int i = 0; i < 3; i++) {
            if (color[i] != 0.0) {
            newColor[i] = (
                dot(normalLightRay, normalPosition) <= 0.0
            )? 0.1*color[i]: color[i]*(0.1 + 1.0*dot(normalLightRay, normalPosition));
        }
    }
    newColor[3] = 1.0;
    return newColor;
}


void main() {
    ZAxisAlignedFrustum f = ZAxisAlignedFrustum(1.0, 1.0, -1.0, 10.0);
    vec4 transformedNormal = rotate(vec4(normal, 1.0), rotationQuaternion);
    vec4 transformedPosition = rotate(scale*pos, rotationQuaternion);
    vTexture = texture;
    vColor = changeColor(color, transformedNormal, vec3(0.0, 0.0, -1.0));
    vPos = transformedPosition;
    vNorm = transformedNormal;
    gl_Position = projectZ(transformedPosition, -2.0, f);
}