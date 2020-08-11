#ifndef _VERTEX_ARRAY
# define _VERTEX_ARRAY


/*
* Vertex structure.
*/
struct vertices {
    int n; // Total number of vertices
    int vertex_size; // Memory allocated to each vertex, in size of floats.
    int position_offset; // position offset
    int normal_offset; // normal offset
    int uv_offset; // uv offset
    int color_offset; // color offset
    int shared_edge; // Whether the edges are shared or not.
    float *_arr; // The actual data.
    int *_edge_arr; // The edge array
};

/*
* Initialize the vertex structure, where
* the parameters are
* v - pointer to the vertex struct
* n - the number of vertices
* arr - pointer to the vertex array data
* edges - pointer to  the edge array data
*/
void vertices_init(struct vertices *v, int n, float *arr, int *edges);


/*
* Get the pointer to the vertex array data.
*/
float *vertices_get_array(const struct vertices *v);


// int valid_vertex_index(struct vertices *v, int i);


/*
* Set the position of the vertex at index i.
*/
void vertices_set_position(struct vertices *v, int i,
                           float x, float y, float z);


/*
* Get the position of the vertex at index i.
*/
float *vertices_get_position(const struct vertices *v, int i);


/*
* Set the normal of the vertex at index i.
*/
void vertices_set_normal(struct vertices *v, int i,
                         float x, float y, float z);
                         

/*
* Get the normal of the vertex at index i.
*/
float *vertices_get_normal(const struct vertices *v, int i);


/*
* Set the uv coordinates of the vertex at index i.
*/
void vertices_set_uv(struct vertices *vert, int i,
                     float u, float v);


/*
* Get the uv coordinates of the vertex at index i.
*/
float *vertices_get_uv(const struct vertices *v, int i);


/*
* Set the colour of the vertex at index i.
*/
void vertices_set_color(struct vertices *v, int i,
                        float r, float b, float g, float a);


/*
* Get the colour of the vertex at index i.
*/
float *vertices_get_color(const struct vertices *v, int i);


/*
* Translate the position of every vertex.
*/
void vertices_translate_position(struct vertices *v, float *pt);


/*
* Get the total size in bytes of the vertex array.
*/
int vertices_array_get_size(const struct vertices *v);


/*
* Make a cylinder where the vertices are not shared.
*/
void make_cylinder(struct vertices *v, 
                   float height, float radius, 
                   int numberOfPointsInEdge);


/*
* Make a sphere where the vertices are not shared.
*/
void make_sphere(struct vertices *v, float radius,
                 int len_half_long, int len_lat);

# endif
