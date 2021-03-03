#version 150 core

#define qu vec4
#define quaternion vec4

attribute vec3 position;
varying vec3 fragPosition;
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

vec3 rotate(vec3 positon, qu r) {
    quaternion qPosition = quaternion(position, 1.0);
    quaternion qPosition2 = mult(qPosition, r);
    r.x *= -1.0;
    r.y *= -1.0;
    r.z *= -1.0;
    return mult(r, qPosition2).xyz;
}

quaternion conjugate(quaternion q) {
    return quaternion(-q.x, -q.y, -q.z, q.w);
}

void main() {
    fragPosition = resize*rotate(position, quaternionAxis);
    fragTexture = rotate(position, quaternionAxis).xy;
    gl_Position = vec4(rotate(position, quaternionAxis).xy, 0.999, 1.0);
}
