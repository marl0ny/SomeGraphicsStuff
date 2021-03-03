#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "vertex_array.h"
// TODO: use better path
#include "../math_helpers/math_functions.h"


/*
* Check if an index is valid. If it isn't
* make the function where this macro is placed
* into return void.
*/
# define VALID_VERTEX_VOID(v, i) do { \
    if (!valid_vertex_index(v, i)) { \
        fprintf(stderr, "Invalid index %d with " \
                "vertex size of %d.\n", (i), (v->n)); \
        return; \
    } \
} while(0)


/*
* Check if an index is valid. If it isn't
* make the function where this macro is placed
* into return null.
*/
# define VALID_VERTEX_NULL(v, i) do { \
    if (!valid_vertex_index(v, i)) { \
        fprintf(stderr, "Invalid index %d with " \
                "vertex size of %d.\n", (i), (v->n)); \
        return NULL; \
    } \
} while(0)


void vertices_init(struct vertices *v, int n, float *arr, int *edges) {
    v->n = n;
    v->vertex_size = 12;
    v->position_offset = 0;
    v->normal_offset = 3;
    v->uv_offset = 6;
    v->color_offset = 8;
    v->shared_edge = 0;
    v->_arr = arr;
    v->_edge_arr = edges;
}

float *vertices_get_array(const struct vertices *v) {
    return v->_arr;
}

int valid_vertex_index(const struct vertices *v, int i) {
    return (i >= 0 && i < v->n);
}


void vertices_set_position(struct vertices *v, int i,
                           float x, float y, float z) {
    VALID_VERTEX_VOID(v, i);
    v->_arr[v->vertex_size*i] = x;
    v->_arr[v->vertex_size*i+1] = y;
    v->_arr[v->vertex_size*i+2] = z;
}


float *vertices_get_position(const struct vertices *v, int i) {
    VALID_VERTEX_NULL(v, i);
    return &(v->_arr[v->vertex_size*i]);
}


void vertices_set_normal(struct vertices *v, int i,
                         float x, float y, float z) {
    VALID_VERTEX_VOID(v, i);
    v->_arr[v->vertex_size*i+3] = x;
    v->_arr[v->vertex_size*i+4] = y;
    v->_arr[v->vertex_size*i+5] = z;
}


float *vertices_get_normal(const struct vertices *v, int i) {
    VALID_VERTEX_NULL(v, i);
    return &(v->_arr[v->vertex_size*i + 3]);
}


void vertices_set_uv(struct vertices *vert, int i,
                     float u, float v) {
    VALID_VERTEX_VOID(vert, i);
    vert->_arr[vert->vertex_size*i+6] = u;
    vert->_arr[vert->vertex_size*i+7] = v;
}


float *vertices_get_uv(const struct vertices *v, int i) {
    VALID_VERTEX_NULL(v, i);
    return &(v->_arr[v->vertex_size*i + 6]);
}


void vertices_set_color(struct vertices *v, int i,
                        float r, float b, float g, float a) {
    VALID_VERTEX_VOID(v, i);
    v->_arr[v->vertex_size*i+8] = r;
    v->_arr[v->vertex_size*i+9] = b;
    v->_arr[v->vertex_size*i+10] = g;
    v->_arr[v->vertex_size*i+11] = a;
}


float *vertices_get_color(const struct vertices *v, int i) {
    VALID_VERTEX_NULL(v, i);
    return &(v->_arr[v->vertex_size*i + 8]);
}


void vertices_translate_position(struct vertices *v, float *pt) {
    for (int i = 0; i < v->n; i++) {
        const float *p0 = vertices_get_position(v, i);
        vertices_set_position(v, i, pt[0] + p0[0], pt[1] + p0[1], pt[2] + p0[2]);
    }
}


int vertices_array_get_size(const struct vertices *v) {
    return v->n*v->vertex_size*sizeof(float);
}


# undef VALID_VERTEX_NULL
# undef VALID_VERTEX_VOID


#define SET_POS_UV_NORM_COL \
vertices_set_position(v, vertex_number, \
r*cos(phi)*sin(theta), \
r*cos(theta),\
r*sin(phi)*sin(theta)); \
sub_inplace3(&norm, (vec3 *)vertices_get_position(v, vertex_number)); \
vertices_set_uv(v, vertex_number, phi/(2.0*PI), 1.0 - theta/PI);\
normalize_inplace3(&norm);\
vertices_set_normal(v, vertex_number, norm.elem[0], norm.elem[1], norm.elem[2]);\
norm.elem[0] = 0.0; norm.elem[1] = 0.0; norm.elem[2] = 0.0; \
vertices_set_color(v, vertex_number++, 0.8, 0.15, 0.15, 1.0)


void make_sphere(struct vertices *v, 
                 float radius,
                 int len_half_long, int len_lat) {
    int circle_len = len_lat;
    int arc_len = len_half_long;
    float r = radius;
    double phi = 0.0, theta = 0.0;
    int vertex_number = 0;
    vec3 norm = init_vec3(0.0, 0.0, 0.0);
    for (int j = 0; j < arc_len; j++) {
        for (int i = 0; i < circle_len; i++) {
            theta = PI*(j + 1)/arc_len;
            phi = 2.0*PI*i/circle_len;
            SET_POS_UV_NORM_COL;
            phi = 2.0*PI*(i+1)/circle_len;
            SET_POS_UV_NORM_COL;
            theta = PI*j/arc_len;
            phi = 2.0*PI*i/circle_len;
            SET_POS_UV_NORM_COL;
            SET_POS_UV_NORM_COL;
            theta = PI*(j+1)/arc_len;
            phi = 2.0*PI*(i+1)/circle_len;
            SET_POS_UV_NORM_COL;
            theta = PI*j/arc_len;
            SET_POS_UV_NORM_COL;
        }
    }
}


#undef SET_POS_UV_NORM_COL

/*
#define SET_NORM_AND_COLOR(v, i, n)\
for (int j = 0; j < n; j++) {\
    vertices_set_color(v, i+j, 0.8, 0.15, 0.15, 1.0);\
}\
float v1[3];\
subtract3(v1, vertices_get_position(v, i), \
            vertices_get_position(v, i+1));\
float v2[3];\
subtract3(v2, vertices_get_position(v, i+1),\
            vertices_get_position(v, i+2));\
float norm[3];\
cross_product(norm, v1, v2);\
normalize3(norm);\
for (int j = 0; j < n; j++) {\
    vertices_set_normal(v, i+j, norm[0], norm[1], norm[2]);\
}\


void make_cylinder(struct vertices *v, 
                   float height, float radius, 
                   int numberOfPointsInEdge) {
    float h = height;
    float r = radius;
    int numberOfVerticesPerSideface = 6;
    for (int i = 0, k = 0; 
         i < numberOfPointsInEdge*numberOfVerticesPerSideface; 
         i+=numberOfVerticesPerSideface, k++) {
        float angle = 2.0*k*PI/(float)numberOfPointsInEdge;
        float c = cosf(angle);
        float s = sinf(angle);
        vertices_set_position(v, i, r*c, h/2, r*s); // top left
        vertices_set_uv(v, i, fmodf(angle, 1.0), 1.0);
        vertices_set_position(v, i+1, r*c, -h/2, r*s); // bottom left
        vertices_set_uv(v, i+1, fmodf(angle, 1.0), 0.0);
        angle = 2.0*(float)(k+1)*PI/(float)numberOfPointsInEdge;
        c = cosf(angle);
        s = sinf(angle);
        vertices_set_position(v, i+2, r*c, -h/2, r*s); // bottom right
        vertices_set_uv(v, i+2, fmodf(angle, 1.0), 0.0);
        vertices_set_position(v, i+3, r*c, -h/2, r*s); // bottom right
        vertices_set_uv(v, i+3, fmodf(angle, 1.0), 0.0);
        vertices_set_position(v, i+4, r*c, h/2, r*s); // top right
        vertices_set_uv(v, i+4, fmodf(angle, 1.0), 1.0);
        angle = 2.0*k*PI/(float)numberOfPointsInEdge;
        c = cosf(angle);
        s = sinf(angle);
        vertices_set_position(v, i+5, r*c, h/2, r*s); // top left
        vertices_set_uv(v, i+5, fmodf(angle, 1.0), 1.0);
        SET_NORM_AND_COLOR(v, i, 6);
    }
    for (int i = numberOfPointsInEdge*numberOfVerticesPerSideface, 
         k = 0; 
         i < 3*numberOfPointsInEdge + 
         numberOfPointsInEdge*numberOfVerticesPerSideface; 
         i+=3, k++) {
        float angle = 2.0*k*PI/(float)numberOfPointsInEdge;
        float c = cosf(angle);
        float s = sinf(angle);
        vertices_set_position(v, i, 0.0, h/2, 0.0); // top centre
        vertices_set_uv(v, i, 0.5, 0.5);
        vertices_set_position(v, i+1, r*c, h/2, r*s); // top left edge
        vertices_set_uv(v, i+1, 0.5*(1.0 + INVSQRT2*c - INVSQRT2*s), 
                        0.5*(1.0 + INVSQRT2*s + INVSQRT2*c));
        angle = 2.0*(float)(k+1)*PI/(float)numberOfPointsInEdge;
        c = cosf(angle);
        s = sinf(angle);
        vertices_set_position(v, i+2, r*c, h/2, r*s); // top right edge
        vertices_set_uv(v, i+2, 0.5*(1.0 + INVSQRT2*c - INVSQRT2*s), 
                        0.5*(1.0 + INVSQRT2*s + INVSQRT2*c));
        SET_NORM_AND_COLOR(v, i, 3);
    }
    for (int i = numberOfPointsInEdge*numberOfVerticesPerSideface
         + 3*numberOfPointsInEdge, 
         k = 0; 
         i < 6*numberOfPointsInEdge + 
         numberOfPointsInEdge*numberOfVerticesPerSideface; 
         i+=3, k++) {
        float angle = 2.0*k*PI/(float)numberOfPointsInEdge;
        float c = cosf(angle);
        float s = sinf(angle);
        vertices_set_position(v, i, r*c, -h/2, r*s); // bottom left edge
        vertices_set_uv(v, i, 0.5*(1.0 + INVSQRT2*c - INVSQRT2*s), 
                        0.5*(1.0 + INVSQRT2*c + INVSQRT2*s));
        vertices_set_position(v, i+1, 0.0, -h/2, 0.0); // bottom centre
        vertices_set_uv(v, i+1, 0.5, 0.5);
        angle = 2.0*(float)(k+1)*PI/(float)numberOfPointsInEdge;
        c = cosf(angle);
        s = sinf(angle);
        vertices_set_position(v, i+2, r*c, -h/2, r*s); // bottom right edge
        vertices_set_uv(v, i+2, 0.5*(1.0 + INVSQRT2*c - INVSQRT2*s), 
                        0.5*(1.0 + INVSQRT2*c + INVSQRT2*s));
        SET_NORM_AND_COLOR(v, i, 3);
    }
}

# undef SET_NORM_AND_COLOR
*/