#include <math.h>
#include "math_functions.h"

static int LEN2 = 2;
static int LEN3 = 3;
static int LEN4 = 4;

#ifndef __MATH_FUNCTIONS__

#define PI 3.141592653589793

#define init_q init_vec4
#define copy_q copy4
#define add_q add4
#define sub_q sub4
#define scale_q scale4
#define normalize_q normalize4
#define copy_inplace_q copy_inplace4
#define add_inplace_q add_inplace4
#define sub_inplace_q sub_inplace4

typedef struct vec2 vec2;
typedef struct vec3 vec3;
typedef struct vec4 vec4;
typedef struct vec4 quaternion;
typedef struct vec4 axis;
typedef struct mat2x2 mat2x2;
typedef struct mat4x4 mat4x4;
typedef float real;


struct vec2 {
    union {
        struct {
            real x;
            real y;
        };
        struct {
            real s;
            real t;
        };
        struct {
            real u;
            real v;
        };
        real elem[2];
    };
};

struct vec3 {
    union {
        struct {
            real x;
            real y;
            real z;
            real _pad_xyz;
        };
        struct {
            real r;
            real g;
            real b;
            real _pad_rgb;
        };
        real elem[4];
    };
};


struct vec4 {
    union {
        struct {
            real x;
            real y;
            real z;
            real w;
        };
        struct {
            real r;
            real g;
            real b;
            real a;
        };
        struct {
            real i;
            real j;
            real k;
            real re;
        };
        struct {
            real x_axis;
            real y_axis;
            real z_axis;
            real angle;
        };
        real elem[4];
    };
};

struct mat2x2 {
    union {
        struct {
            real elem[4];
        };
        struct {
            vec2 rows[2];
        };
    };
};

struct mat4x4 {
    union {
        struct {
            real elem[16];
        };
        struct {
            vec4 rows[4];
        };
    };
};


#endif

real sum(const real *arr, int len) {
    real sum = 0.0;
    do {
        sum += arr[len-1];
    }while(--len);
    return sum;
}

real abs2(const real *arr, int len) {
    real sum = 0.0;
    do {
        sum += arr[len-1]*arr[len-1];
    }while(--len);
    return sum;
}

real sqrt_abs2(const real *arr, int len) {
    return sqrt(abs2(arr, len));
}


vec2 init_vec2(real x, real y) {
    vec2 w;
    w.elem[0] = x;
    w.elem[1] = y;
    return w;
}

vec2 copy2(const vec2 *v) {
    vec2 w = init_vec2(v->elem[0], v->elem[1]);
    return w;
}


vec2 add2(const vec2 *v1, const vec2 *v2) {
    vec2 w;
    for (int i = 0; i < LEN2; i++) {
        w.elem[i] = v1->elem[i] + v2->elem[i];
    }
    return w;
}


vec2 sub2(const vec2 *v1, const vec2 *v2) {
    vec2 w;
    for (int i = 0; i < LEN2; i++) {
        w.elem[i] = v1->elem[i] - v2->elem[i];
    }
    return w;
}

vec2 mul2(const vec2 *v1, const vec2 *v2) {
    vec2 w;
    for (int i = 0; i < LEN2; i++) {
        w.elem[i] = v1->elem[i] * v2->elem[i];
    }
    return w;
}

vec2 div2(const vec2 *v1, const vec2 *v2) {
    vec2 w;
    for (int i = 0; i < LEN2; i++) {
        w.elem[i] = v1->elem[i] / v2->elem[i];
    }
    return w;
}

vec2 scale2(real a, const vec2 *v) {
    vec2 w;
    for (int i = 0; i < LEN2; i++) {
        w.elem[i] = a*v->elem[i];
    }
    return w;
}

vec2 normalize2(const vec2 *v) {
    vec2 w;
    real v_norm = sqrt_abs2((real *)v, LEN2);
    for (int i = 0; i < LEN2; i++) {
        w.elem[i] = v->elem[i]/v_norm;
    }
    return w;
}

void copy_inplace2(vec2 *v, const vec2 *v2) {
    for (int i = 0; i < LEN2; i++) {
        v->elem[i] = v2->elem[i];
    }
}

void add_inplace2(vec2 *v, const vec2 *v2) {
    for (int i = 0; i < LEN2; i++) {
        v->elem[i] += v2->elem[i];
    }
}

void sub_inplace2(vec2 *v, const vec2 *v2) {
    for (int i = 0; i < LEN2; i++) {
        v->elem[i] -= v2->elem[i];
    }
}

void mul_inplace2(vec2 *v, const vec2 *v2) {
    for (int i = 0; i < LEN2; i++) {
        v->elem[i] *= v2->elem[i];
    }
}

void div_inplace2(vec2 *v, const vec2 *v2) {
    for (int i = 0; i < LEN2; i++) {
        v->elem[i] /= v2->elem[i];
    }
}

void normalize_inplace2(vec2 *v) {
    real v_norm = sqrt_abs2((real *)v, LEN2);
    for (int i = 0; i < LEN2; i++) {
        v->elem[i] = v->elem[i]/v_norm;
    }
}

void scale_inplace2(vec2 *v, real a) {
    for (int i = 0; i < LEN2; i++) {
        v->elem[i] = a*v->elem[i];
    }
}

real dot2(const vec2 *v1, const vec2* v2) {
    real sum = 0.0;
    for (int i = 0; i < LEN2; i++) {
        sum += v1->elem[i]*v2->elem[i];
    }
    return sum;
}

// 3D

vec3 init_vec3(real x, real y, real z) {
    vec3 w;
    w.elem[0] = x;
    w.elem[1] = y;
    w.elem[2] = z;
    w.elem[3] = 1.0;
    return w;
}

vec3 copy3(const vec3 *v) {
    vec3 w = init_vec3(v->elem[0], v->elem[1],
                    v->elem[2]);
    return w;
}


vec3 add3(const vec3 *v1, const vec3 *v2) {
    vec3 w;
    for (int i = 0; i < LEN3; i++) {
        w.elem[i] = v1->elem[i] + v2->elem[i];
    }
    return w;
}


vec3 sub3(const vec3 *v1, const vec3 *v2) {
    vec3 w;
    for (int i = 0; i < LEN3; i++) {
        w.elem[i] = v1->elem[i] - v2->elem[i];
    }
    return w;
}

vec3 mul3(const vec3 *v1, const vec3 *v2) {
    vec3 w;
    for (int i = 0; i < LEN3; i++) {
        w.elem[i] = v1->elem[i] * v2->elem[i];
    }
    return w;
}

vec3 div3(const vec3 *v1, const vec3 *v2) {
    vec3 w;
    for (int i = 0; i < LEN3; i++) {
        w.elem[i] = v1->elem[i] / v2->elem[i];
    }
    return w;
}

vec3 scale3(real a, const vec3 *v) {
    vec3 w;
    for (int i = 0; i < LEN3; i++) {
        w.elem[i] = a*v->elem[i];
    }
    return w;
}

vec3 normalize3(const vec3 *v) {
    vec3 w;
    real v_norm = sqrt_abs2((real *)v, LEN3);
    for (int i = 0; i < LEN3; i++) {
        w.elem[i] = v->elem[i]/v_norm;
    }
    return w;
}

void copy_inplace3(vec3 *v, const vec3 *v2) {
    for (int i = 0; i < LEN3; i++) {
        v->elem[i] = v2->elem[i];
    }
}

void add_inplace3(vec3 *v, const vec3 *v2) {
    for (int i = 0; i < LEN3; i++) {
        v->elem[i] += v2->elem[i];
    }
}

void sub_inplace3(vec3 *v, const vec3 *v2) {
    for (int i = 0; i < LEN3; i++) {
        v->elem[i] -= v2->elem[i];
    }
}

void mul_inplace3(vec3 *v, const vec3 *v2) {
    for (int i = 0; i < LEN3; i++) {
        v->elem[i] *= v2->elem[i];
    }
}

void div_inplace3(vec3 *v, const vec3 *v2) {
    for (int i = 0; i < LEN3; i++) {
        v->elem[i] /= v2->elem[i];
    }
}

void normalize_inplace3(vec3 *v) {
    real v_norm = sqrt_abs2((real *)v, LEN3);
    for (int i = 0; i < LEN3; i++) {
        v->elem[i] = v->elem[i]/v_norm;
    }
}

void scale_inplace3(vec3 *v, real a) {
    for (int i = 0; i < LEN3; i++) {
        v->elem[i] = a*v->elem[i];
    }
}

real dot3(const vec3 *v1, const vec3* v2) {
    real sum = 0.0;
    for (int i = 0; i < LEN3; i++) {
        sum += v1->elem[i]*v2->elem[i];
    }
    return sum;
}

vec4 init_vec4(real x, real y, real z, real w) {
    vec4 v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
}

vec4 copy4(const vec4 *v) {
    vec4 w = init_vec4(v->elem[0], v->elem[1],
                    v->elem[2], v->elem[3]);
    return w;
}


vec4 add4(const vec4 *v1, const vec4 *v2) {
    vec4 w;
    for (int i = 0; i < LEN4; i++) {
        w.elem[i] = v1->elem[i] + v2->elem[i];
    }
    return w;
}


vec4 sub4(const vec4 *v1, const vec4 *v2) {
    vec4 w;
    for (int i = 0; i < LEN4; i++) {
        w.elem[i] = v1->elem[i] - v2->elem[i];
    }
    return w;
}

vec4 mul4(const vec4 *v1, const vec4 *v2) {
    vec4 w;
    for (int i = 0; i < LEN4; i++) {
        w.elem[i] = v1->elem[i] * v2->elem[i];
    }
    return w;
}

vec4 div4(const vec4 *v1, const vec4 *v2) {
    vec4 w;
    for (int i = 0; i < LEN4; i++) {
        w.elem[i] = v1->elem[i] / v2->elem[i];
    }
    return w;
}

vec4 scale4(real a, const vec4 *v) {
    vec4 w;
    for (int i = 0; i < LEN4; i++) {
        w.elem[i] = a*v->elem[i];
    }
    return w;
}

vec4 normalize4(const vec4 *v) {
    vec4 w;
    real v_norm = sqrt_abs2((real *)v, LEN4);
    for (int i = 0; i < LEN4; i++) {
        w.elem[i] = v->elem[i]/v_norm;
    }
    return w;
}

void copy_inplace4(vec4 *v, const vec4 *v2) {
    for (int i = 0; i < LEN4; i++) {
        v->elem[i] = v2->elem[i];
    }
}

void add_inplace4(vec4 *v, const vec4 *v2) {
    for (int i = 0; i < LEN4; i++) {
        v->elem[i] += v2->elem[i];
    }
}

void sub_inplace4(vec4 *v, const vec4 *v2) {
    for (int i = 0; i < LEN4; i++) {
        v->elem[i] -= v2->elem[i];
    }
}

void mul_inplace4(vec4 *v, const vec4 *v2) {
    for (int i = 0; i < LEN4; i++) {
        v->elem[i] *= v2->elem[i];
    }
}

void div_inplace4(vec4 *v, const vec4 *v2) {
    for (int i = 0; i < LEN4; i++) {
        v->elem[i] /= v2->elem[i];
    }
}

void normalize_inplace4(vec4 *v) {
    real v_norm = sqrt_abs2((real *)v, LEN4);
    for (int i = 0; i < LEN4; i++) {
        v->elem[i] = v->elem[i]/v_norm;
    }
}

void scale_inplace4(vec4 *v, real a) {
    for (int i = 0; i < LEN4; i++) {
        v->elem[i] = a*v->elem[i];
    }
}

real dot4(const vec4 *v1, const vec4* v2) {
    real sum = 0.0;
    for (int i = 0; i < LEN4; i++) {
        sum += v1->elem[i]*v2->elem[i];
    }
    return sum;
}

quaternion mul_q(const quaternion *q1, const quaternion *q2) {
    quaternion q3;
    q3.re = q1->re*q2->re - q1->i*q2->i - q1->j*q2->j - q1->k*q2->k;
    q3.i = q1->i*q2->re + q1->re*q2->i + q1->j*q2->k - q1->k*q2->j;
    q3.j = q1->j*q2->re + q1->re*q2->j + q1->k*q2->i - q1->i*q2->k;
    q3.k = q1->k*q2->re + q1->re*q2->k + q1->i*q2->j - q1->j*q2->i;
    return q3;
}


quaternion rotate_q(quaternion *r, const quaternion *q) {
    conjugate_inplace_q(r);
    quaternion tmp = mul_q(q, r);
    conjugate_inplace_q(r);
    quaternion qf = mul_q(r, &tmp);
    return qf;
}

quaternion conjugate_q(const quaternion *q) {
    quaternion q2;
    q2.re = q->re;
    q2.i = -q->i;
    q2.j = -q->j;
    q2.k = -q->k;
    return q2;
}

void conjugate_inplace_q(quaternion *q) {
    q->i *= -1.0;
    q->j *= -1.0;
    q->k *= -1.0;
}

axis init_axis(real x, real y, real z, real angle) {
    real norm = sqrt(x*x + y*y + z*z);
    axis a;
    a.x_axis = x/norm;
    a.y_axis = y/norm;
    a.z_axis = z/norm;
    a.angle = angle;
    return a;
}

quaternion rotation_axis_q(axis *a) {
    real c = cos(a->angle/2.0);
    real s = sin(a->angle/2.0);
    return init_q(s*a->x_axis, s*a->y_axis, s*a->z_axis, c);
}

vec2 matmul2(const struct mat2x2 *m, const vec2 *v) {
    vec2 v2;
    for (int i = 0; i < LEN2; i++) {
        v2.elem[i] = dot2(&m->rows[i], v);
    }
    return v2;
}

vec4 matmul4(const struct mat4x4 *m, const vec4 *v) {
    vec4 v2;
    for (int i = 0; i < LEN4; i++) {
        v2.elem[i] = dot4(&m->rows[i], v);
    }
    return v2;
}

mat2x2 identity2x2() {
    mat2x2 m;
    vec2 row0 = init_vec2(1.0, 0.0);
    vec2 row1 = init_vec2(0.0, 1.0);
    copy_inplace2(&m.rows[0], &row0);
    copy_inplace2(&m.rows[1], &row1);
    return m;
}

mat4x4 identity4x4() {
    mat4x4 m;
    vec4 row0 = init_vec4(1.0, 0.0, 0.0, 0.0);
    vec4 row1 = init_vec4(0.0, 1.0, 0.0, 0.0);
    vec4 row2 = init_vec4(0.0, 0.0, 1.0, 0.0);
    vec4 row3 = init_vec4(0.0, 0.0, 0.0, 1.0);
    copy_inplace4(&m.rows[0], &row0);
    copy_inplace4(&m.rows[1], &row1);
    copy_inplace4(&m.rows[2], &row2);
    copy_inplace4(&m.rows[3], &row3);
    return m;
}

mat4x4 z_rotation4x4(real theta) {
    mat4x4 m = identity4x4();
    m.rows[0].elem[0] = cos(theta);
    m.rows[0].elem[1] = -sin(theta);
    m.rows[1].elem[0] = sin(theta);
    m.rows[1].elem[1] = cos(theta);
    return m;
}

mat4x4 y_rotation4x4(real theta) {
    mat4x4 m = identity4x4();
    m.rows[0].elem[0] = cos(theta);
    m.rows[0].elem[2] = -sin(theta);
    m.rows[2].elem[0] = sin(theta);
    m.rows[2].elem[2] = cos(theta);
    return m;
}

mat4x4 x_rotation4x4(real theta) {
    mat4x4 m = identity4x4();
    m.rows[1].elem[1] = cos(theta);
    m.rows[1].elem[2] = -sin(theta);
    m.rows[2].elem[1] = sin(theta);
    m.rows[2].elem[2] = cos(theta);
    return m;
}

#ifdef __TEST__
#include <stdio.h>

void test() {
    real arr1[2] = {4.0, 3.0};
    printf("%f\n", sum(arr1, 2));
    printf("%f\n", sqrt_abs2(arr1, 2));
    printf("%f\n", abs2(arr1, 2));
    vec2 v = init_vec2(4.0, 2.0);
    vec2 v2 = init_vec2(-2.0, 5.5);
    // 2.0, 7.5
    printf("%f %f\n", add2(&v, &v2).x, add2(&v, &v2).y);
    // 6.0, -3.5
    printf("%f %f\n", sub2(&v, &v2).x, sub2(&v, &v2).y);
    // -8.0, 11.0
    printf("%f %f\n", mul2(&v, &v2).x, mul2(&v, &v2).y);
    // -2.0, 0.363636...
    printf("%f %f\n", div2(&v, &v2).x, div2(&v, &v2).y);
    // 40.0, 20.0
    printf("%f %f\n", scale2(10.0, &v).x, scale2(10.0, &v).y);

    vec2 v3 = init_vec2(3.0, 4.0);
    // 0.6, 0.8
    printf("%f %f\n", normalize2(&v3).x, normalize2(&v3).y);
    // 3.0, 4.0
    printf("%f %f\n", copy2(&v3).x, copy2(&v3).y);
    // -2.0, 5.5
    copy_inplace2(&v3, &v2);
    printf("%f %f\n", copy2(&v3).x, copy2(&v3).y);
    // -1.0, 6.5
    vec2 tmp = init_vec2(1.0, 1.0);
    add_inplace2(&v3, &tmp);
    printf("%f %f\n", copy2(&v3).x, copy2(&v3).y);
    // 1.0, -1.0
    tmp.x = -2.0; tmp.y = 7.5;
    sub_inplace2(&v3, &tmp);
    printf("%f %f\n", copy2(&v3).x, copy2(&v3).y);
     // 3.0, -4.0
    tmp.x = 3.0; tmp.y = 4.0;
    mul_inplace2(&v3, &tmp);
    printf("%f %f\n", copy2(&v3).x, copy2(&v3).y);
    // 0.3333..., -1.0
    tmp.x = 9.0; tmp.y = 4.0;
    div_inplace2(&v3, &tmp);
    printf("%f %f\n", copy2(&v3).x, copy2(&v3).y);
    // -0.6, 0.8
    v3.s = -3.0;
    v3.t = 4.0;
    normalize_inplace2(&v3);
    printf("%f %f\n", v3.s, v3.t);
    // -60.0, 80.0
    scale_inplace2(&v3, 100.0);
    printf("%f %f\n", v3.u, v3.v);
    // 3.0
    printf("%f\n", dot2(&v, &v2));
    // 0.707107, 0.707107, 0.0, 1.0
    vec4 v4 = init_vec4(1.0, 0.0, 0.0, 1.0);
    mat4x4 m4 = z_rotation4x4(3.14159/4.0);
    vec4 v42 = matmul4(&m4, &v4);
    printf("%f %f %f %f\n", v42.x, v42.y, v42.z, v42.w);
    // 0.707107, 0.707107, 0.0
    axis a = init_axis(0.0, 0.0, 1.0, 3.14159/4.0);
    quaternion qr = rotation_axis_q(&a);
    quaternion q = init_q(1.0, 0.0, 0.0, 1.0);
    printf("%f %f %f\n", rotate_q(&qr, &q).i, rotate_q(&qr, &q).j,
            rotate_q(&qr, &q).k);
     // 0.0, 0.0, 0.866025, 0.5
    axis a2 = init_axis(1.0, 0.0, 0.0, 3.14159/6.0);
    quaternion qr2 = rotation_axis_q(&a2);
    quaternion q2 = init_q(0.0, 1.0, 0.0, 1.0);
    printf("%f %f %f\n", rotate_q(&qr2, &q2).i, rotate_q(&qr2, &q2).j,
            rotate_q(&qr2, &q2).k);
}

#else

void test() {}

#endif


