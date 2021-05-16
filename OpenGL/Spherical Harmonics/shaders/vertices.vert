#version 330 core

#define qu vec4
#define quaternion vec4

attribute vec3 position;
attribute vec3 normal;
attribute vec3 colour;
attribute vec2 texture;
varying vec3 fragPosition;
varying vec3 fragColour;
varying vec3 fragNormal;
varying vec2 fragTexture;
uniform vec4 quaternionAxis;
uniform float resize;


quaternion mult(qu p, qu q) {
    return quaternion(
        p.w*q.x + p.x*q.w + p.y*q.z - p.z*q.y,
        p.w*q.y + p.y*q.w + p.z*q.x - p.x*q.z,
        p.w*q.z + p.z*q.w + p.x*q.y - p.y*q.x,
        p.w*q.w - p.x*q.x - p.y*q.y - p.z*q.z
    );
}

quaternion conjugate(quaternion q) {
    return quaternion(-q.x, -q.y, -q.z, q.w);
}

vec3 rotate(vec3 positon, qu r) {
    quaternion qPosition = quaternion(position, 1.0);
    quaternion rInv = conjugate(r);
    return mult(rInv, mult(qPosition, r)).xyz;
}

vec4 zProject(vec3 p0, float zSource, 
              float nearPlaneWidth, float nearPlaneHeight, 
              float zNear, float zFar) {
    float z = p0.z - zSource;
    zNear = zNear - zSource;
    zFar = zFar - zSource;
    return vec4(
        zNear*p0.x/(nearPlaneWidth/2.0),
        zNear*p0.y/(nearPlaneHeight/2.0),
        -(zFar + zNear)/(zFar - zNear)*z + (2.0*zFar*zNear)/(zFar - zNear),
        -z
    );

}

void main() {
    fragPosition = resize*rotate(position, quaternionAxis);
    fragNormal = rotate(normal, quaternionAxis);
    fragColour = vec3(colour);
    fragTexture = vec2(texture);
    // gl_Position = vec4(fragPosition, 1.0);
    gl_Position = zProject(fragPosition, 2.0, 1.0, 1.0, 1.0, -4.0);
}
