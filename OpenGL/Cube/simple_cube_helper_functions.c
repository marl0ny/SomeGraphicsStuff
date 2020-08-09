#include <stdlib.h>
#include <stdio.h>
#include <math.h>


char *get_file_contents(char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        perror("fopen");
        fclose(f);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    int file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *file_buff = malloc(file_size + 1);
    if (file_buff == NULL) {
        perror("malloc");
        fclose(f);
        return NULL;
    }
    fread(file_buff, file_size, 1, f);
    file_buff[file_size] = '\0';
    fclose(f);
    return file_buff;
}


void rotation_transform(float *rotation_array,
                   double a1, double a2, double a3){
    double pi = 3.14159;
    a1 *= pi;
    a2 *= pi;
    a3 *= pi;
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


void transform(float *transform_array, float *vector) {
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


void quaternion_multiply_by(float *q1, float *q2) {
    float q3[4];
    q3[0] = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
    q3[1] = q1[1]*q2[0] + q1[0]*q2[1] + q1[2]*q2[3] - q1[3]*q2[2];
    q3[2] = q1[2]*q2[0] + q1[0]*q2[2] + q1[3]*q2[1] - q1[1]*q2[3];
    q3[3] = q1[3]*q2[0] + q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1];
    for (int i = 0; i < 4; i++) {
        q1[i] = q3[i];
    }
}


void axis_rotation(float *q_axis, float angle, float *axis) {
    float norm = axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2];
    norm = sqrt(norm);
    for (int i = 0; i < 3; i++) {
        axis[i] = axis[i]/norm;
    }
    float c = (float)cos(angle/2);
    float s = (float)sin(angle/2);
    q_axis[0] = c;
    for (int i = 0; i < 3; i++) {
        q_axis[i+1] = s*axis[i];
    }
}


void cross_product(float *c, float *a, float *b) {
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = - a[0]*b[2] + a[2]*b[0];
    c[2] = a[0]*b[1] - a[1]*b[0];
}
