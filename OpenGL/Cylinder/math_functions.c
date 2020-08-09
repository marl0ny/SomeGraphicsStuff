#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "math_functions.h"


void add3(float *c, const float *a, const float *b) {
    c[0] = a[0] + b[0];
    c[1] = a[1] + b[1];
    c[2] = a[2] + b[2];
}


void add_inplace3(float *c, const float *a) {
    c[0] += a[0];
    c[1] += a[1];
    c[2] += a[2];
}


void add4(float *c, const float *a, const float *b) {
    c[0] = a[0] + b[0];
    c[1] = a[1] + b[1];
    c[2] = a[2] + b[2];
    c[3] = a[3] + b[3];
}


void add_inplace4(float *c, const float *a) {
    c[0] += a[0];
    c[1] += a[1];
    c[2] += a[2];
    c[3] += a[3];
}


void subtract3(float *c, const float *a, const float *b) {
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
}


void subtract_inplace3(float *c, const float *a) {
    c[0] -= a[0];
    c[1] -= a[1];
    c[2] -= a[2];
}


void subtract4(float *c, const float *a, const float *b) {
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
    c[3] = a[3] - b[3];
}


void subtract_inplace4(float *c, const float *a) {
    c[0] -= a[0];
    c[1] -= a[1];
    c[2] -= a[2];
    c[3] -= a[3];
}


void normalize3(float *v) {
    float norm = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    for (int i = 0; i < 3; i++) {
        v[i] = v[i]/norm;
    }
}


float dot3(float c, const float *a, const float *b) {
    float dot_val;
    for (int i = 0; i < 3; i++) {
        dot_val += a[i]*b[i];
    }
    return dot_val;
}


void cross_product(float *c, const float *a, const float *b) {
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = - a[0]*b[2] + a[2]*b[0];
    c[2] = a[0]*b[1] - a[1]*b[0];
}


void transform_inplace4(float *transform_array, float *vector) {
    float temp[4];
    for (int i = 0; i < 4; i++) {
        temp[i] = vector[i];
    }
    for (int i = 0; i < 4; i++) {
        vector[i] = 0.0f;
        for (int j = 0; j < 4; j++) {
            vector[j] += transform_array[i*4 + j]*temp[j];
        }
    }
}


void rotation_transform4(float *rotation_array,
                   double a1, double a2, double a3){
    a1 *= PI;
    a2 *= PI;
    a3 *= PI;
    rotation_array[    0] = (float)(cos(a1)*cos(a3)-cos(a2)*sin(a1)*sin(a3));
    rotation_array[    1] = (float)(-cos(a1)*sin(a3)-cos(a2)*cos(a3)*sin(a1));
    rotation_array[    2] = (float)(sin(a1)*sin(a2));
    rotation_array[    3] = 0.0F;
    rotation_array[4    ] = (float)(cos(a3)*sin(a1)+cos(a1)*cos(a2)*sin(a3));
    rotation_array[4 + 1] = (float)(cos(a1)*cos(a2)*cos(a3)-sin(a1)*sin(a3));
    rotation_array[4 + 2] = (float)(-cos(a1)*sin(a2));
    rotation_array[4 + 3] = 0.0F;
    rotation_array[8    ] = (float)(sin(a2)*sin(a3));
    rotation_array[8 + 1] = (float)(cos(a3)*sin(a2));
    rotation_array[8 + 2] = (float)(cos(a2));
    rotation_array[8 + 3] = 0.0F;
    rotation_array[12    ] = 0.0F;
    rotation_array[12 + 1] = 0.0F;
    rotation_array[12 + 2] = 0.0F;
    rotation_array[12 + 3] = 1.0F;
}


void quaternion_multiply(float *q3, const float *q1, const float *q2) {
    q3[0] = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
    q3[1] = q1[1]*q2[0] + q1[0]*q2[1] + q1[2]*q2[3] - q1[3]*q2[2];
    q3[2] = q1[2]*q2[0] + q1[0]*q2[2] + q1[3]*q2[1] - q1[1]*q2[3];
    q3[3] = q1[3]*q2[0] + q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1];
}


void quaternion_multiply_inplace(float *q1, const float *q2) {
    float q3[4];
    q3[0] = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
    q3[1] = q1[1]*q2[0] + q1[0]*q2[1] + q1[2]*q2[3] - q1[3]*q2[2];
    q3[2] = q1[2]*q2[0] + q1[0]*q2[2] + q1[3]*q2[1] - q1[1]*q2[3];
    q3[3] = q1[3]*q2[0] + q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1];
    for (int i = 0; i < 4; i++) {
        q1[i] = q3[i];
    }
}


void quaternion_rotation_axis(float *q_axis, 
                              float angle, float *axis) {
    float norm = axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2];
    norm = sqrt(norm);
    for (int i = 0; i < 3; i++) {
        axis[i] = axis[i]/norm;
    }
    float c = (float)cos(angle/2.0);
    float s = (float)sin(angle/2.0);
    q_axis[0] = c;
    for (int i = 0; i < 3; i++) {
        q_axis[i+1] = s*axis[i];
    }
}


void quaternion_normalize(float *q2, const float *q1) {
    float length = sqrtf(
        q1[0]*q1[0] + q1[1]*q1[1] + q1[2]*q1[2] + q1[3]*q1[3]);
    q2[0] = q1[0]/length;
    q2[1] = q1[1]/length;
    q2[2] = q1[2]/length;
    q2[3] = q1[3]/length;
}


void quaternion_normalize_inplace(float *q) {
    float length = sqrtf(
        q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    q[0] = q[0]/length;
    q[1] = q[1]/length;
    q[2] = q[2]/length;
    q[3] = q[3]/length;
}


void quaternion_conjugate(float *q2, const float *q1) {
    q2[0] = q1[0];
    q2[1] = -q1[1];
    q2[2] = -q1[2];
    q2[3] = -q1[3];
}


void quaternion_conjugate_inplace(float *q) {
    q[1] = -q[1];
    q[2] = -q[2];
    q[3] = -q[3];
}


void quaternion_invert(float *q2, const float *q1) {
    float length = sqrtf(
        q1[0]*q1[0] + q1[1]*q1[1] + q1[2]*q1[2] + q1[3]*q1[3]);
    q2[0] = q1[0]/length;
    q2[1] = -q1[1]/length;
    q2[2] = -q1[2]/length;
    q2[3] = -q1[3]/length;
}


void quaternion_invert_inplace(float *q) {
    float length = sqrtf(
        q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    q[0] = q[0]/length;
    q[1] = -q[1]/length;
    q[2] = -q[2]/length;
    q[3] = -q[3]/length;
}


void quaternion_rotate(float *qf, const float *r, const float *q0) {
    float q0r[4];
    quaternion_multiply(q0r, q0, r);
    float invr[4];
    quaternion_conjugate(invr, r);
    quaternion_multiply(qf, invr, q0r);
}


void quaternion_rotate3(float *vf, const float *r, const float *v0) {
    float q[4] = {0.0};
    q[0] = 1.0;
    q[1] = v0[0];
    q[2] = v0[1];
    q[3] = v0[2];
    float qf[4];
    quaternion_rotate(qf, r, q);
    vf[0] = qf[1];
    vf[1] = qf[2];
    vf[2] = qf[3];
}

