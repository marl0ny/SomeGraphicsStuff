#ifdef __cplusplus
extern "C" {
#endif
#ifndef __MATH_FUNCTIONS__
#define __MATH_FUNCTIONS__

#define PI 3.141592653589793
#define INVSQRT2 0.7071067811865475

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


real sum(const real *arr, int len) ;
real abs2(const real *arr, int len) ;
real sqrt_abs2(const real *arr, int len) ;
vec2 init_vec2(real x, real y) ;
vec2 copy2(const vec2 *v) ;
vec2 add2(const vec2 *v1, const vec2 *v2) ;
vec2 sub2(const vec2 *v1, const vec2 *v2) ;
vec2 mul2(const vec2 *v1, const vec2 *v2) ;
vec2 div2(const vec2 *v1, const vec2 *v2) ;
vec2 scale2(real a, const vec2 *v) ;
vec2 normalize2(const vec2 *v) ;
void copy_inplace2(vec2 *v, const vec2 *v2) ;
void add_inplace2(vec2 *v, const vec2 *v2) ;
void sub_inplace2(vec2 *v, const vec2 *v2) ;
void mul_inplace2(vec2 *v, const vec2 *v2) ;
void div_inplace2(vec2 *v, const vec2 *v2) ;
void normalize_inplace2(vec2 *v) ;
void scale_inplace2(vec2 *v, real a) ;
real dot2(const vec2 *v1, const vec2* v2) ;
vec3 init_vec3(real x, real y, real z) ;
vec3 copy3(const vec3 *v) ;
vec3 add3(const vec3 *v1, const vec3 *v2) ;
vec3 sub3(const vec3 *v1, const vec3 *v2) ;
vec3 mul3(const vec3 *v1, const vec3 *v2) ;
vec3 div3(const vec3 *v1, const vec3 *v2) ;
vec3 scale3(real a, const vec3 *v) ;
vec3 normalize3(const vec3 *v) ;
void copy_inplace3(vec3 *v, const vec3 *v2) ;
void add_inplace3(vec3 *v, const vec3 *v2) ;
void sub_inplace3(vec3 *v, const vec3 *v2) ;
void mul_inplace3(vec3 *v, const vec3 *v2) ;
void div_inplace3(vec3 *v, const vec3 *v2) ;
void normalize_inplace3(vec3 *v) ;
void scale_inplace3(vec3 *v, real a) ;
real dot3(const vec3 *v1, const vec3* v2) ;
vec4 init_vec4(real x, real y, real z, real w) ;
vec4 copy4(const vec4 *v) ;
vec4 add4(const vec4 *v1, const vec4 *v2) ;
vec4 sub4(const vec4 *v1, const vec4 *v2) ;
vec4 mul4(const vec4 *v1, const vec4 *v2) ;
vec4 div4(const vec4 *v1, const vec4 *v2) ;
vec4 scale4(real a, const vec4 *v) ;
vec4 normalize4(const vec4 *v) ;
void copy_inplace4(vec4 *v, const vec4 *v2) ;
void add_inplace4(vec4 *v, const vec4 *v2) ;
void sub_inplace4(vec4 *v, const vec4 *v2) ;
void mul_inplace4(vec4 *v, const vec4 *v2) ;
void div_inplace4(vec4 *v, const vec4 *v2) ;
void normalize_inplace4(vec4 *v) ;
void scale_inplace4(vec4 *v, real a) ;
real dot4(const vec4 *v1, const vec4* v2) ;
quaternion mul_q(const quaternion *q1, const quaternion *q2) ;
quaternion rotate_q(quaternion *r, const quaternion *q) ;
quaternion conjugate_q(const quaternion *q) ;
void conjugate_inplace_q(quaternion *q) ;
axis init_axis(real x, real y, real z, real angle) ;
quaternion rotation_axis_q(axis *a) ;
vec2 matmul2(const struct mat2x2 *m, const vec2 *v) ;
vec4 matmul4(const struct mat4x4 *m, const vec4 *v) ;
mat2x2 identity2x2() ;
mat4x4 identity4x4() ;
mat4x4 z_rotation4x4(real theta) ;
mat4x4 y_rotation4x4(real theta) ;
mat4x4 x_rotation4x4(real theta) ;

#endif
#ifdef __cplusplus
}
#endif
