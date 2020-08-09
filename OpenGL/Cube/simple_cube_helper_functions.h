#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#ifndef _SIMPLE_CUBE_HELPER_FUNCTIONS
#define _SIMPLE_CUBE_HELPER_FUNCTIONS 


/*
* Given a filename, get its file contents.
*/
char *get_file_contents(char *filename);


/* 
* Compute the rotation transformation, 
* for angles a1, a2, and a3.
* These angles are measured in radians/pi.
*/
void rotation_transform(float *rotation_array, double a1, double a2, double a3);


void transform(float *transform_array, float *vector);


void quaternion_multiply_by(float *q1, float *q2);


void axis_rotation(float *q_axis, float angle, float *axis);


void cross_product(float *c, float *a, float *b);


#endif