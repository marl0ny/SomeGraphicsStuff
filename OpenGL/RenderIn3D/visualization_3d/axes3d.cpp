#include "axes3d.hpp"

#include <vector>

static std::vector<float> get_x_line_vertices() {
    return {};
}

static std::vector<float> get_y_line_vertices() {
    return {};
}

static std::vector<float> get_z_line_vertices() {
    return {};
}

static std::vector<float> get_axes_vertices() {
    return std::vector{
        0.0F, 0.0F, 0.0F,
        1.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F,
        0.0F, 0.0F, 1.0F
    };
}

static std::vector<int> get_axes_elements() {
    return {0, 1, 0, 2, 0, 3};
}

WireFrame axes3d::get_axes_wireframe() {
    std::vector<int> elements = get_axes_elements();
    std::vector<float> vertices = get_axes_vertices();
    return WireFrame(
        {{"position", Attribute{
                .size=3, .type=GL_FLOAT, .normalized=false,
                .stride=0, .offset=0}}
        }, vertices, elements, WireFrame::LINES);
}
