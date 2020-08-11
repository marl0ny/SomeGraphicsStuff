

#ifndef _MATH_FUNCTIONS
#define _MATH_FUNCTIONS 


static const float PI = 3.1415926535897932384626433832795F;
static const float INVSQRT2 = 0.7071067811865475F;


/*
* Add 3 elements of a with the corresponding 3 elements in b,
* and store the result in c.
*/
void add3(float *c, const float *a, const float *b);


/* 
* Add the first 3 elements of c inplace with the first
* 3 elements of a.
*/
void add_inplace3(float *c, const float *a);


/*
* Add 4 elements of a with the corresponding 4 elements in b,
* and store the result in c.
*/
void add4(float *c, const float *a, const float *b);


/* 
* Add the first 4 elements of c inplace with the first
* 4 elements of a.
*/
void add_inplace4(float *c, const float *a);


/*
* Subtract 3 elements of a with the corresponding 3 elements in b,
* and store the result in c.
*/
void subtract3(float *c, const float *a, const float *b);


/* 
* Subtract the first 3 elements of c inplace with the first
* 3 elements of a.
*/
void subtract_inplace3(float *c, const float *a);


/*
* Subtract 4 elements of a with the corresponding 4 elements in b,
* and store the result in c.
*/
void subtract4(float *c, const float *a, const float *b);


/* 
* Subtract the first 4 elements of c inplace with the first
* 4 elements of a.
*/
void subtract_inplace4(float *c, const float *a);


/*
* Normalize a 3 element array vector inplace.
*/
void normalize3(float *v);


/*
* Compute the dot product in 3D of a with b and store the
* result in c. 
*/
float dot3(float c, const float *a, const float *b);


/*
* Compute the cross product of a with b
* and store the result in c.
*/
void cross_product(float *c, const float *a, const float *b);


/*
* Transform a vector wth four elements inplace by 
* the transformation array.
*/
void transform_inplace4(float *transform_array, float *vector);


/* 
* Compute the rotation transformation, 
* for angles a1, a2, and a3.
* These angles are measured in radians/pi.
*/
void rotation_transform4(float *rotation_array, 
                         double a1, double a2, double a3);


/*
* Multiply the quaternions q1 and q2 together inplace and store
* the result in q3.
*/
void quaternion_multiply(float *q3, const float *q1, const float *q2);


/*
* Multiply the quaternion q1 inplace by q2.
*/
void quaternion_multiply_inplace(float *q1, const float *q2);


/*
* Given an angle and axis of rotation, compute the equivalent
* quaternion. The axis gets normalized in the process.
*/
void quaternion_rotation_axis(float *q_axis, 
                              float angle, float *axis);


/*
* Normalize the quaternion q1 and store the result in q2.
*/
void quaternion_normalize(float *q2, const float *q1);


/*
* Normalize the quaternion q inplace.
*/
void quaternion_normalize_inplace(float *q);


/*
* Take the conjugate of the quaternion q and store
* the result in q2.
*/
void quaternion_conjugate(float *q2, const float *q1);


/*
* Take the conjugate of the quaternion q inplace.
*/
void quaternion_conjugate_inplace(float *q);


/*
* Invert the quaternion q1 and store the result in q2.
*/
void quaternion_invert(float *q2, const float *q1);


/*
* Invert the quaternion q inplace.
*/
void quaternion_invert_inplace(float *q);


/*
* Using a rotation quaternion r, rotate the quaternion q0 and store
* the result in qf. It is assumed that r is a valid rotation
* quaternion (already normalized).
*/
void quaternion_rotate(float *qf, const float *r, const float *q0);


/*
* Using a rotation quaternion r, rotate 3 vector array v0 and store
* the result in vf. It is assumed that r is a valid rotation
* quaternion (already normalized).
*/
void quaternion_rotate3(float *vf, const float *r, const float *v0);


#endif