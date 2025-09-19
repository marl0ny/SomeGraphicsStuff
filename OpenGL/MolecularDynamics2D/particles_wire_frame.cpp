/* The layout of circle vertices are:
    (circle_center_x, circle_center_y, angle, type).
circle_center_x and circle_center_y gives the texture coordinate
which they will sample to. Angle gives the angular offset
of an edge vertex. The variable type is used to designate this
vertex as either a center vertex, where in this case the value of type
is set to zero, or as a vertex along the circle's edge,
where here it is set to 1. The actual radii of the
edge vertices from their center and their overal spatial position
are determined in the vertex shader step: the
circle_center_x and circle_center_y are used to sample from a texture
that contains the actual positions of each circle, and uniform values
are used to set their radii.
*/
#include "particles_wire_frame.hpp"

#define PI 3.141592653589793

enum {
    IN_CENTER=0, ALONG_EDGE=1  
};

static int
add_circle_center_vertex_to_vertices_get_its_index(
    std::vector<float> &vertices, int &vertex_count, 
    float center_tex_coord_u, float center_tex_coord_v) {
    vertices.push_back(center_tex_coord_u);
    vertices.push_back(center_tex_coord_v);
    vertices.push_back(0.0);
    vertices.push_back((float)IN_CENTER);
    int circle_center_index = vertex_count;
    vertex_count++;
    return circle_center_index;
}

static std::vector<int>
add_edge_circle_vertices_to_vertices_get_their_indices(
    std::vector<float> &vertices, int &vertex_count, 
    float center_tex_coord_u, float center_tex_coord_v, 
    int number_of_edge_points
) {
    std::vector<int> edge_elements(number_of_edge_points);
    for (int k = 0; k < number_of_edge_points; k++) {
        double angle = double(k)*2.0*PI/(double)number_of_edge_points;
        vertices.push_back(center_tex_coord_u);
        vertices.push_back(center_tex_coord_v);
        vertices.push_back(angle);
        vertices.push_back((float)ALONG_EDGE);
        edge_elements[k] = vertex_count;
        vertex_count++;
    }
    return edge_elements;
}

static void add_circle_indices_to_elements(
    std::vector<int> &elements,
    int circle_center_index, std::vector<int> edge_indices
) {
    for (int k = 0; k < edge_indices.size(); k++) {
        int triangle_element[3] = {
            circle_center_index, edge_indices[k], 
            edge_indices[(k+1) % edge_indices.size()]
        };
        for (int t = 0; t < 3; t++)
            elements.push_back(triangle_element[t]);
    }
}

static void set_circles_vertices_elements(
    IVec2 d_2d, std::vector<float> &vertices, std::vector<int> &elements) {
    int vertex_count = 0;
    int number_of_edge_points = 12;
    for (int i = 0; i < d_2d[1]; i++) {
        for (int j = 0; j < d_2d[0]; j++) {
            float u = (float(j) + 0.5F)/float(d_2d[0]);
            float v = (float(i) + 0.5F)/float(d_2d[1]);
            int circle_center_index = 
                add_circle_center_vertex_to_vertices_get_its_index(
                 vertices, vertex_count, u, v);
            std::vector<int> edge_indices
                = add_edge_circle_vertices_to_vertices_get_their_indices(
                    vertices, vertex_count,
                    u, v,
                    number_of_edge_points);
            add_circle_indices_to_elements(
                elements, circle_center_index, edge_indices);
        }        
    }
}

WireFrame get_particles_wire_frame(IVec2 d_2d) {
    Attributes attributes = {
        {"position", {
            .size=4, .type=GL_FLOAT, .normalized=false, .stride=0, .offset=0
    }}};
    std::vector<float> vertices {};
    std::vector<int> elements {};
    set_circles_vertices_elements(d_2d, vertices, elements);
    return WireFrame(attributes, vertices, elements, WireFrame::TRIANGLES);
}
