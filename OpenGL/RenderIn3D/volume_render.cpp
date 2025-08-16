#include "volume_render.hpp"

using namespace volume_render;

static std::vector<float> get_cube_outline_vertices_set_elements(
    std::vector<int> &elements
) {
    std::vector<float> vertices {
        -1.0, -1.0, -1.0, // 0 - bottom left
        1.0, -1.0, -1.0, // 1 - bottom right
        1.0, 1.0, -1.0, // 2 - upper right
        -1.0, 1.0, -1.0, // 3 - upper left
        -1.0, -1.0, 1.0, // 4
        1.0, -1.0, 1.0, // 5
        1.0, 1.0, 1.0, // 6
        -1.0, 1.0, 1.0, // 7
    };
    elements = {
        0, 1, 1, 2, 2, 3, 3, 0,
        0, 4, 3, 7, 2, 6, 1, 5,
        4, 5, 5, 6, 6, 7, 7, 4
    };
    // for (auto &e: elements) {
    //     printf("%g ", vertices[3*e]);
    //     printf("%g ", vertices[3*e + 1]);
    //     printf("%g\n", vertices[3*e + 2]);
    // }
    return vertices;

}

#include <iostream>

static std::vector<float> get_vertices_set_elements(
    std::vector<int> &elements,
    IVec2 volume_texel_dimensions2d,
    IVec3 volume_texel_dimensions3d
) {
    int width = volume_texel_dimensions3d[0];
    int height = volume_texel_dimensions3d[1];
    int length = volume_texel_dimensions3d[2];
    int element_size = 4;
    std::vector<float> vertices (2*element_size*length);
    for (int i = 0; i < length; i++) {
        int j = length - i - 1;
        Vec3 uvw_bottom_left = Vec3{.ind{
            0.5F/float(width), 
            0.5F/float(height), 
            (float(i) + 0.5F)/float(length)
        }};
        Vec3 uvw_bottom_right = Vec3{.ind{
            (float(width) - 0.5F)/float(width), 
            0.5F/float(height), 
            (float(i) + 0.5F)/float(length)
        }};
        Vec3 uvw_upper_right = Vec3{.ind{
            (float(width) - 0.5F)/float(width), 
            (float(height) - 0.5F)/float(height), 
            (float(i) + 0.5F)/float(length)
        }};
        Vec3 uvw_upper_left = Vec3{.ind{
            0.5F/float(width), 
            (float(height) - 0.5F)/float(height), 
            (float(i) + 0.5F)/float(length)
        }};
        Vec2 uv_bottom_left = get_2d_from_3d_texture_coordinates(
            uvw_bottom_left,
            volume_texel_dimensions2d, 
            volume_texel_dimensions3d);
        Vec2 uv_bottom_right = get_2d_from_3d_texture_coordinates(
            uvw_bottom_right,
            volume_texel_dimensions2d, 
            volume_texel_dimensions3d);
        Vec2 uv_upper_right = get_2d_from_3d_texture_coordinates(
            uvw_upper_right,
            volume_texel_dimensions2d, 
            volume_texel_dimensions3d);
        Vec2 uv_upper_left = get_2d_from_3d_texture_coordinates(
            uvw_upper_left,
            volume_texel_dimensions2d, 
            volume_texel_dimensions3d);
        {
            int x = 0, y = 1;
            vertices[2*(element_size*j) + x] = uv_bottom_left.x;
            vertices[2*(element_size*j) + y] = uv_bottom_left.y;
            vertices[2*(element_size*j + 1) + x] = uv_bottom_right.x;
            vertices[2*(element_size*j + 1) + y] = uv_bottom_right.y;
            vertices[2*(element_size*j + 2) + x] = uv_upper_right.x;
            vertices[2*(element_size*j + 2) + y] = uv_upper_right.y;
            vertices[2*(element_size*j + 3) + x] = uv_upper_left.x;
            vertices[2*(element_size*j + 3) + y] = uv_upper_left.y;
        }
        elements.push_back(element_size*j);  // bottom left
        elements.push_back(element_size*j + 1);  // bottom right
        elements.push_back(element_size*j + 2);  // upper right
        elements.push_back(element_size*j + 2);  // upper right
        elements.push_back(element_size*j + 3);  // upper left
        elements.push_back(element_size*j);  // bottom left
    }
    // for (auto &e: elements) {
    //     printf("%g ", vertices[e*2]);
    //     printf("%g ", vertices[e*2 + 1]);
    //     printf("\n");
    // }
    return vertices;
}

static WireFrame get_volume_render_wire_frame(
    IVec2 volume_texel_dimensions2d,
    IVec3 volume_texel_dimensions3d
) {
    std::vector<int> elements{};
    std::vector<float> vertices = get_vertices_set_elements(
        elements, volume_texel_dimensions2d, volume_texel_dimensions3d);
    return WireFrame({
            {"uvIndex", Attribute{
            .size=2, .type=GL_FLOAT, .normalized=false,
            .stride=0, .offset=0}}
        },
        vertices, elements, WireFrame::TRIANGLES
    );
}

static WireFrame get_cube_outline_wire_frame(
) {
    std::vector<int> elements{};
    std::vector<float> vertices = get_cube_outline_vertices_set_elements(
        elements);
    return WireFrame(
        {{"position", Attribute{
                .size=3, .type=GL_FLOAT, .normalized=false,
                .stride=0, .offset=0}}
        }, vertices, elements, WireFrame::LINES);
}

Programs::Programs() {
    this->copy = Quad::make_program_from_path(
        "./shaders/util/copy.frag"
    );
    this->sample_data = Quad::make_program_from_path(
        "./shaders/vol-render/sample.frag"
    );
    this->gradient = Quad::make_program_from_path(
        "./shaders/gradient/gradient3d.frag"
    );
    this->show_volume = make_program_from_paths(
        "./shaders/vol-render/display.vert",
        "./shaders/vol-render/display.frag"
    );
    this->sample_data_show_volume = make_program_from_paths(
        "./shaders/vol-render/display.vert",
        "./shaders/vol-render/sample-display.frag"
    );
    this->cube_outline = make_program_from_paths(
        "./shaders/vol-render/cube-outline.vert",
        "./shaders/util/uniform-color.frag"
    );
    this->zero_boundaries_3d = Quad::make_program_from_path(
        "./shaders/util/zero-boundaries-3d.frag"
    );

}

Frames::Frames(
    IVec2 view_dimensions2d,
    IVec2 data_texel_dimensions2d,
    IVec2 volume_texel_dimensions2d,
    IVec3 volume_texel_dimensions3d,
    unsigned int min_filter,
    unsigned int mag_filter) :
    volume_f16(
        {
            .format=GL_RGBA16F,
            .width=(uint32_t)volume_texel_dimensions2d[0],
            .height=(uint32_t)volume_texel_dimensions2d[1],
            .generate_mipmap=1,
            .wrap_s=GL_CLAMP_TO_EDGE,
            .wrap_t=GL_CLAMP_TO_EDGE,
            .min_filter=min_filter,
            .mag_filter=mag_filter
        }
    ),
    volume_f32(
        {
            .format=GL_RGBA32F,
            .width=(uint32_t)volume_texel_dimensions2d[0],
            .height=(uint32_t)volume_texel_dimensions2d[1],
            .generate_mipmap=1,
            .wrap_s=GL_CLAMP_TO_EDGE,
            .wrap_t=GL_CLAMP_TO_EDGE,
            .min_filter=min_filter,
            .mag_filter=mag_filter
        }
    ),
    data_f16(
        {
            .format=GL_RGBA16F,
            .width=(uint32_t)data_texel_dimensions2d[0],
            .height=(uint32_t)data_texel_dimensions2d[1],
            .generate_mipmap=1,
            .wrap_s=GL_CLAMP_TO_EDGE,
            .wrap_t=GL_CLAMP_TO_EDGE,
            .min_filter=min_filter,
            .mag_filter=mag_filter
        }
    ),
    data_f32(
        {
            .format=GL_RGBA32F,
            .width=(uint32_t)data_texel_dimensions2d[0],
            .height=(uint32_t)data_texel_dimensions2d[1],
            .generate_mipmap=1,
            .wrap_s=GL_CLAMP_TO_EDGE,
            .wrap_t=GL_CLAMP_TO_EDGE,
            .min_filter=min_filter,
            .mag_filter=mag_filter
        }
    ),
    view_tex(
        {
            .format=GL_RGBA16F,
            .width=(uint32_t)view_dimensions2d[0],
            .height=(uint32_t)view_dimensions2d[1],
            .generate_mipmap=1,
            .wrap_s=GL_CLAMP_TO_EDGE,
            .wrap_t=GL_CLAMP_TO_EDGE,
            .min_filter=min_filter,
            .mag_filter=mag_filter
        }
    ),
    data(data_f32),
    gradient_data(data_f32),
    data_half_precision(data_f16),
    gradient_data_half_precision(data_f16),
    volume(volume_f16),
    volume_grad(volume_f32),
    view(view_tex),
    wire_frame(get_volume_render_wire_frame(
        volume_texel_dimensions2d,
        volume_texel_dimensions3d)),
    cube_outline(get_cube_outline_wire_frame())
 {

}

void Frames::reset_data(IVec2 data_dimensions2d) {
    this->data_f32.width = data_dimensions2d[0];
    this->data_f32.height = data_dimensions2d[1];
    this->data_f16.width = data_dimensions2d[0];
    this->data_f16.height = data_dimensions2d[1];
    this->data.reset(this->data_f32);
    this->gradient_data.reset(this->data_f32);
    this->data_half_precision.reset(this->data_f16);
    this->gradient_data_half_precision.reset(this->data_f16);
}

void Frames
::reset_volume(IVec2 volume_dimensions2d, IVec3 volume_dimensions3d) {
    this->volume_f32.width = volume_dimensions2d[0];
    this->volume_f32.height = volume_dimensions2d[1];
    this->volume_f16.width = volume_dimensions2d[0];
    this->volume_f16.height = volume_dimensions2d[1];
    this->volume.reset(this->volume_f16);
    this->volume_grad.reset(this->volume_f32);
    this->wire_frame = WireFrame(
        get_volume_render_wire_frame(
        volume_dimensions2d,
        volume_dimensions3d)
    );
}

// void Frames::reset_view(IVec2 view_dimensions2d) {
//     this->view_tex = {
//         .format=GL_RGBA16F,
//         .width=(uint32_t)view_dimensions2d[0],
//         .height=(uint32_t)view_dimensions2d[1],
//         .generate_mipmap=1,
//         .wrap_s=GL_CLAMP_TO_EDGE,
//         .wrap_t=GL_CLAMP_TO_EDGE,
//         .min_filter=GL_LINEAR,
//         .mag_filter=GL_LINEAR
//     }
//     this->view.reset(view_tex);
// }

static void gradient(Quad &dst, const Quad &volume_data, 
                     uint32_t gradient_program,
                     int order_of_accuracy,
                     int boundary_type,
                     int staggered_mode,
                     int index,
                     IVec3 texel_dimensions3d,
                     IVec2 texel_dimensions2d) {
    dst.draw(
        gradient_program,
        {
            {"tex", &volume_data},
            {"orderOfAccuracy", int(order_of_accuracy)},
            {"boundaryType", int(boundary_type)},
            {"staggeredMode", int(staggered_mode)},
            {"index", int(index)},
            {"texelDimensions2D", texel_dimensions2d},
            {"texelDimensions3D", texel_dimensions3d},
            {"dr", Vec3{.x=1.0, .y=1.0, .z=1.0}},
            {"dimensions3D", Vec3{
                .x=float(texel_dimensions3d[0]),
                .y=float(texel_dimensions3d[1]),
                .z=float(texel_dimensions3d[2])
            }},
        }
    );
}

static void sample_data(
    Quad &dst,
    const Quad &src_data,
    uint32_t sample_data_program,
    float view_scale,
    Quaternion rotation,
    IVec3 volume_texel_dimensions3d,
    IVec2 volume_texel_dimensions2d,
    IVec3 data_texel_dimensions3d,
    IVec2 data_texel_dimensions2d
    ) {
    dst.draw(
        sample_data_program,
        {
            {"tex", &src_data},
            {"viewScale", float(view_scale)},
            {"rotation", rotation},
            {"volumeTexelDimensions3D", volume_texel_dimensions3d},
            {"dataTexelDimensions3D", data_texel_dimensions3d},
            {"volumeTexelDimensions2D", volume_texel_dimensions2d},
            {"dataTexelDimensions2D", data_texel_dimensions2d},
        }
    );
}

static void display_volume(
    RenderTarget &dst,
    uint32_t volume_render_program,
    Uniforms uniforms,
    WireFrame &volume_render_wire_frame
) {
    glEnable(GL_DEPTH_TEST);
    // TODO: check the next line
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    dst.draw(
        volume_render_program,
        uniforms,
        volume_render_wire_frame
    );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}

VolumeRender
::VolumeRender(TextureParams default_tex_params, 
               IVec3 volume_dimensions3d,
               IVec3 data_texel_dimensions3d) :
    view_dimensions(IVec2{
        .x=(int)default_tex_params.width,
        .y=(int)default_tex_params.height}),
    data_texel_dimensions2d(
        get_2d_from_3d_dimensions(data_texel_dimensions3d)),
    volume_texel_dimensions2d(
        get_2d_from_3d_dimensions(volume_dimensions3d)
    ),
    data_texel_dimensions3d(data_texel_dimensions3d),
    volume_texel_dimensions3d(volume_dimensions3d),
    programs(),
    frames(
        this->view_dimensions, 
        data_texel_dimensions2d, 
        volume_texel_dimensions2d,
        volume_dimensions3d,
        default_tex_params.min_filter,
        default_tex_params.mag_filter)
    {
    this->debug_rotation = {.real=1.0, .i=0.0, .j=0.0, .k=0.0};
    this->bounding_box = {
        Vec3{.ind{-1.0, -1.0, -1.0}}, // 0 - bottom left
        Vec3{.ind{1.0, -1.0, -1.0}}, // 1 - bottom right
        Vec3{.ind{1.0, 1.0, -1.0}}, // 2 - upper right
        Vec3{.ind{-1.0, 1.0, -1.0}}, // 3 - upper left
        Vec3{.ind{-1.0, -1.0, 1.0}}, // 4
        Vec3{.ind{1.0, -1.0, 1.0}}, // 5
        Vec3{.ind{1.0, 1.0, 1.0}}, // 6
        Vec3{.ind{-1.0, 1.0, 1.0}}, // 7
    };
}

void VolumeRender
::reset_data_dimensions(IVec3 texel_data_dimensions3d) {
    this->data_texel_dimensions3d = texel_data_dimensions3d;
    IVec2 texel_data_dimensions2d 
        = get_2d_from_3d_dimensions(texel_data_dimensions3d);
    this->data_texel_dimensions2d = texel_data_dimensions2d;
    this->frames.reset_data(texel_data_dimensions2d);
}

void VolumeRender::reset_volume_dimensions(
    IVec3 volume_dimensions3d
) {
    IVec2 volume_dimensions2d 
        = get_2d_from_3d_dimensions(volume_dimensions3d);
    this->volume_texel_dimensions2d = volume_dimensions2d;
    this->volume_texel_dimensions3d = volume_dimensions3d;
    this->frames.reset_volume(volume_dimensions2d, volume_dimensions3d);
}

enum BoundaryType {
    USE_TEXTURE_WRAPPING = 0,
    DIRICHLET=1, DIRICHLET_MASK=2
};


const RenderTarget &VolumeRender
::view(
    const Quad &src_data, float scale, Quaternion rotation,
    float alpha_brightness, float color_brightness,
    Uniforms additional_uniforms
) {
    float max_x = 0.0, max_y = 0.0, max_z = 0.0;
    for (auto &e: this->bounding_box) {
        Quaternion r = rotate(Quaternion{1.0, e.x, e.y, e.z}, rotation);
        float x = r.i, y = r.j, z = r.k;
        max_x = (x > max_x)? x: max_x;
        max_y = (y > max_y)? y: max_y;
        max_z = (z > max_z)? z: max_z;

    }
    float rot_scale = (scale > 1.0)?
        scale: 1.0/std::max(max_x, std::max(max_y, max_z));
    // IVec2 src_data_dimensions = {
    //     .ind{(int)src_data.width(), (int)src_data.height()}};
    // // if (src_data_dimensions[0] != this->data_texel_dimensions2d[0] ||
    // //     src_data_dimensions[1] != this->data_texel_dimensions2d[1]) {
    // //    // this->data_texel_dimensions2d[0]
    // //     this->frames.reset_data(src_data_dimensions);
    // // }
    this->frames.data_half_precision.draw(
        this->programs.zero_boundaries_3d,
        {
            {"tex", &src_data},
            {"texelDimensions2D", this->data_texel_dimensions2d},
            {"texelDimensions3D", this->data_texel_dimensions3d}
        }
    );
    gradient(
        this->frames.gradient_data,
        this->frames.data_half_precision,
        this->programs.gradient,
        2,
        BoundaryType::USE_TEXTURE_WRAPPING,
        0,
        3,
        data_texel_dimensions3d,
        data_texel_dimensions2d);
    this->frames.volume.clear();
    this->frames.volume_grad.clear();
    sample_data( 
        this->frames.volume,
        this->frames.data_half_precision,
        this->programs.sample_data,
        rot_scale,
        rotation,
        this->volume_texel_dimensions3d,
        this->volume_texel_dimensions2d,
        this->data_texel_dimensions3d,
        this->data_texel_dimensions2d
    );
    sample_data(
        this->frames.volume_grad,
        this->frames.gradient_data,
        this->programs.sample_data,
        rot_scale,
        rotation,
        this->volume_texel_dimensions3d,
        this->volume_texel_dimensions2d,
        this->data_texel_dimensions3d,
        this->data_texel_dimensions2d
    );
    this->frames.view.clear();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    this->frames.view.draw(
        this->programs.cube_outline,
        {
            {"rotation", rotation},
            {"viewScale", scale},
            {"color", Vec4{.ind{1.0, 1.0, 1.0, 0.5}}}
        },
        this->frames.cube_outline
    );
    // glDisable(GL_BLEND);
    Uniforms view_uniforms = {
        {"colorBrightness", color_brightness},
        {"alphaBrightness", alpha_brightness}
    };
    if (!additional_uniforms.empty())
        for (auto &e: additional_uniforms)
            view_uniforms.insert(e);
    view_uniforms.insert({"rotation", rotation});
    view_uniforms.insert({"gradientTex", &this->frames.volume_grad});
    view_uniforms.insert({"densityTex", &this->frames.volume});
    view_uniforms.insert(
        {"fragmentTexelDimensions3D", this->volume_texel_dimensions3d});
    view_uniforms.insert(
        {"fragmentTexelDimensions2D", this->volume_texel_dimensions2d});
    view_uniforms.insert(
        {"texelDimensions2D", this->volume_texel_dimensions2d});
    view_uniforms.insert(
        {"texelDimensions3D", this->volume_texel_dimensions3d});
    view_uniforms.insert({"debugRotation", this->debug_rotation});
    view_uniforms.insert({"debugShow2DTexture", int(0)});
    view_uniforms.insert({"scale", float(scale/rot_scale)});
    display_volume(this->frames.view, 
        this->programs.show_volume,
        view_uniforms,
        this->frames.wire_frame);
    return this->frames.view;
}

const Quad &VolumeRender::get_gradient_half_precision() const {
    return this->frames.gradient_data_half_precision;
}

const Quad &VolumeRender::volume_quad() const {
    return this->frames.volume;
}

const Quad &VolumeRender::volume_gradient_quad() const {
    return this->frames.volume_grad;
}