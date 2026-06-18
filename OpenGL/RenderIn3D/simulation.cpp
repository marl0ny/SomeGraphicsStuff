#include "simulation.hpp"
#include "cube_outline.hpp"
#include "cursor_outline3d.hpp"
#include "axes3d.hpp"
#include "bmp.hpp"

#include <iostream>

using namespace sim_3d;


static const std::vector<float> QUAD_VERTICES = {
    -1.0, -1.0, 0.0, -1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, -1.0, 0.0};
static const std::vector<int> QUAD_ELEMENTS = {0, 1, 2, 0, 2, 3};
static WireFrame get_quad_wire_frame() {
    return WireFrame(
        {{"position", Attribute{
            3, GL_FLOAT, false,
            0, 0}}},
        QUAD_VERTICES, QUAD_ELEMENTS,
        WireFrame::TRIANGLES
    );
}

Programs::Programs() {
    this->domain_color = Quad::make_program_from_path(
        "./shaders/vol-render/domain-coloring.frag"
    );
    this->copy = Quad::make_program_from_path(
        "./shaders/util/copy.frag"
    );
    this->gradient = Quad::make_program_from_path(
        "./shaders/gradient/gradient3d.frag"
    );
    this->blur = Quad::make_program_from_path(
        "./shaders/util/blur.frag"
    );
    this->cube_outline = make_program_from_paths(
        "./shaders/cube-outline/cube-outline.vert",
        "./shaders/util/uniform-color.frag"
    );
    this->cursor_outline = make_program_from_paths(
        "./shaders/cursor/outline3d.vert",
        "./shaders/util/uniform-color.frag"
    );
    this->axes_3d = make_program_from_paths(
        "./shaders/axes/axes3d.vert",
        "./shaders/axes/axes3d.frag"
    );
    this->axes_labels_3d = make_program_from_paths(
        "./shaders/axes/axes-labels3d.vert",
        "./shaders/axes/axes3d.frag"
    );
    this->user_defined = 0;
}

Frames::
Frames(const TextureParams &default_tex_params, const SimParams &params):
    data_tex_params({
        .format=GL_RGBA32F,
        .width=(unsigned int)get_2d_from_3d_dimensions(
            params.dataTexelDimensions3D)[0],
        .height=(unsigned int)get_2d_from_3d_dimensions(
            params.dataTexelDimensions3D)[1],
        .generate_mipmap=default_tex_params.generate_mipmap,
        .min_filter=default_tex_params.min_filter,
        .mag_filter=default_tex_params.mag_filter,
        .wrap_s=GL_REPEAT,
        .wrap_t=GL_REPEAT
    }),
    data(data_tex_params),
    tmp(data_tex_params),
    render_tmp(default_tex_params),
    // render_tmp2(default_tex_params),
    render(default_tex_params),
    quad_wire_frame(get_quad_wire_frame()),
    arrows3d_frame(
        line_arrows3d::get_3d_vector_field_wire_frame(
            params.arrowDimensions)),
    conical_arrows3d_frame(
        conical_arrows3d::get_3d_vector_field_wire_frame(
            params.arrowDimensions, 13)) {
}

void Frames::reset_data_dimensions(IVec3 texel_dimensions3d) {
    IVec2 texel_dimensions2d = get_2d_from_3d_dimensions(texel_dimensions3d);
    this->data_tex_params.width = texel_dimensions2d[0];
    this->data_tex_params.height = texel_dimensions2d[1];
    data.reset(this->data_tex_params);
    tmp.reset(this->data_tex_params);
}

static Vec3 scale_rotate(Vec3 r, float scale, Quaternion rotation) {
    Vec3 r2 = r - Vec3{.x=0.5, 0.5, 0.0};
    Quaternion q = rotate(
        Quaternion{.real=1.0, r2.x, r2.y, r2.z},
        rotation.conj());
    return Vec3{.x=q.i/scale, q.j/scale, q.k/scale};
}

static void take_screenshot(
    const SimParams &params, RenderTarget &render,
    std::vector<unsigned char> &image_data,
    std::vector<unsigned char> &image_rgba_arr) {
    if (params.takeScreenshots.is_recording) {
        BMPHeader header (
            params.takeScreenshots.width, 
            params.takeScreenshots.height);
        memcpy(
            (unsigned char *)&image_data[0], 
            &header, sizeof(BMPHeader));
        render.fill_array_with_contents(
            (unsigned char *)&image_rgba_arr[0]);
        for (int i = 0; i < params.takeScreenshots.height; i++) {
            for (int j = 0; j < params.takeScreenshots.width; j++) {
                // unsigned char a = image_rgba_arr[
                //     4*(i*params.takeScreenshots.width + j)];
                unsigned char r = image_rgba_arr[
                    4*(i*params.takeScreenshots.width + j) + 2];
                unsigned char g = image_rgba_arr[
                    4*(i*params.takeScreenshots.width + j) + 1];
                unsigned char b = image_rgba_arr[
                    4*(i*params.takeScreenshots.width + j)];
                image_data[
                    54 + 3*(i*params.takeScreenshots.width + j)
                ] = r;
                image_data[
                    54 + 3*(i*params.takeScreenshots.width + j) + 1
                ] = g;
                image_data[
                    54 + 3*(i*params.takeScreenshots.width + j) + 2
                ] = b;
            }
        }
        BMPHeader *header_ptr = (BMPHeader *)(&image_data[0]);
        int max_val = 0;
        for (int i = 54; i < image_data.size(); i++)
            max_val = (image_data[i] > max_val)? image_data[i]: max_val;
        printf("Max val: %d\n", max_val);
        print_bmp_header(*header_ptr);
    }
}

Simulation::
Simulation(const TextureParams &default_tex_params, const SimParams &params
) : m_volume_render(default_tex_params,
    params.volumeTexelDimensions3D,
    params.dataTexelDimensions3D),
    m_planar_slices(default_tex_params),
    m_arrows3d(params.arrowDimensions, default_tex_params),
    m_conical_arrows3d(params.arrowDimensions, 13, default_tex_params),
    m_frames(default_tex_params, params) {
    m_image_data = std::vector<unsigned char>(
        54 + get_bmp_row_byte_size(params.takeScreenshots.width)
        *params.takeScreenshots.height, 0
    );
    m_image_rgba_arr = std::vector<unsigned char>(
        4*params.takeScreenshots.width*params.takeScreenshots.height, 0);
}

const RenderTarget &Simulation
::view(
    const SimParams &params,
    const std::optional<Vec2> &hover,
    ::Quaternion rotation, float scale) {
    enum {VOL_RENDER_VIEW=0, PLANAR_SLICES_VIEW=1, VECTOR_FIELD_VIEW=2, 
        PLANR_SLICES_VECTOR_FIELD_VIEW=3, VOL_RENDER_VECTOR_FIELD_VIEW=4};
    switch(params.visualizationSelect.selected) {
        case PLANAR_SLICES_VIEW: case PLANR_SLICES_VECTOR_FIELD_VIEW: {
            this->m_frames.render.clear();
            this->m_frames.render_tmp.clear();
            Vec2 scaled_hover;
            if (hover.has_value()) {
                IVec2 tex_dims = m_frames.render.texture_dimensions();
                scaled_hover = Vec2{
                    .x=hover->x,
                    .y=hover->y*(float(tex_dims[1])/float(tex_dims[0]))
                        + 0.5F*(1.0F - float(tex_dims[1])/float(tex_dims[0])) 
                };
            }
            m_planar_slices.view(
                this->m_frames.render,
                this->m_frames.data, params.dataTexelDimensions3D,
                rotation, scale,
                int(params.dataTexelDimensions3D.z
                    *params.planarNormCoordOffsets[0]),
                int(params.dataTexelDimensions3D.x
                    *params.planarNormCoordOffsets[1]),
                int(params.dataTexelDimensions3D.y
                    *params.planarNormCoordOffsets[2]),
                (hover.has_value())? scaled_hover: Vec2{.ind {0.0, 0.0}},
                params.usePerspectiveProjection
            );
            this->m_cursor_location = m_planar_slices.most_perpendicular_intersection(
                params.dataTexelDimensions3D,
                rotation, scale,
                int(params.dataTexelDimensions3D.z
                    *params.planarNormCoordOffsets[0]),
                int(params.dataTexelDimensions3D.x
                    *params.planarNormCoordOffsets[1]),
                int(params.dataTexelDimensions3D.y
                    *params.planarNormCoordOffsets[2]),
                (hover.has_value())? scaled_hover: Vec2{.ind {0.0, 0.0}}
            );
            {
                IVec3 arrows_d3d = params.arrowDimensions;
                IVec2 arrows_d2d = get_2d_from_3d_dimensions(arrows_d3d);
                IVec3 tex_d3d = params.dataTexelDimensions3D;
                IVec2 tex_d2d = get_2d_from_3d_dimensions(tex_d3d);
                Vec3 dr = Vec3{
                    .x=params.simulationDimensions3D.x/tex_d3d.x,
                    .y=params.simulationDimensions3D.y/tex_d3d.y,
                    .z=params.simulationDimensions3D.z/tex_d3d.z};
                this->m_frames.tmp.draw(
                    m_programs.gradient,
                    {
                        {"tex", &m_frames.data},
                        {"orderOfAccuracy", int(4)},
                        {"staggeredMode", int(0)},
                        {"index", int(0)},
                        {"texelDimensions3D", tex_d3d},
                        {"texelDimensions2D", tex_d2d},
                        {"dr", dr},
                        {"dimensions3D", params.simulationDimensions3D}

                    }
                );

                this->m_frames.render_tmp.clear();
                // this->m_frames.render.clear();
                if (params.visualizationSelect.selected 
                        == PLANR_SLICES_VECTOR_FIELD_VIEW) {
                    if (params.useCones) {
                        m_conical_arrows3d.view(
                            this->m_frames.render, this->m_frames.tmp,
                            2.0*scale, rotation, 
                            params.arrowDimensions,
                            params.dataTexelDimensions3D,
                            {
                                {"useOrthogonalProjection",
                                        (params.usePerspectiveProjection)? 
                                        int(0): int(1)},
                                {"rescaleZ", int(0)}
                            });
                    } else {
                        m_arrows3d.view(
                            this->m_frames.render, this->m_frames.tmp,
                            2.0*scale, rotation, 
                            params.arrowDimensions,
                            params.dataTexelDimensions3D,
                            {
                                {"useOrthogonalProjection",
                                        (params.usePerspectiveProjection)? 
                                        int(0): int(1)},
                                {"rescaleZ", int(0)}
                            });
                    }
                }
                
            }
            WireFrame axes = axes3d::get_axes_wireframe();
            WireFrame axes_labels = axes3d::get_xyz_axes_labels_wireframe();
            axes3d::draw_axes(
                this->m_frames.render,
                {.axes=m_programs.axes_3d, .labels=m_programs.axes_labels_3d},
                axes, axes_labels,
                rotation, 110, 0.0F,
                params.usePerspectiveProjection, 
                m_frames.render.texture_dimensions());
            /*glDisable(GL_DEPTH_TEST);
            WireFrame cube_outline = get_cube_outline_wire_frame();
            this->m_frames.render.draw(
                m_programs.cube_outline,
                {
                    {"rotation", rotation},
                    {"viewScale", scale},
                    {"color", Vec4{.ind{1.0, 1.0, 1.0, 0.5}}},
                    {"usePerspectiveProjection", int(0)},
                    {"rescaleZ", int(0)}
                },
                cube_outline
            );*/
            take_screenshot(
                params, m_frames.render, 
                m_image_data, m_image_rgba_arr);
            return m_frames.render;
        }
        case VECTOR_FIELD_VIEW: {
            IVec3 arrows_d3d = params.arrowDimensions;
            IVec2 arrows_d2d = get_2d_from_3d_dimensions(arrows_d3d);
            IVec3 tex_d3d = params.dataTexelDimensions3D;
            IVec2 tex_d2d = get_2d_from_3d_dimensions(tex_d3d);
            Vec3 dr = Vec3{
                .x=params.simulationDimensions3D.x/tex_d3d.x,
                .y=params.simulationDimensions3D.y/tex_d3d.y,
                .z=params.simulationDimensions3D.z/tex_d3d.z};
            this->m_frames.tmp.draw(
                m_programs.gradient,
                {
                    {"tex", &m_frames.data},
                    {"orderOfAccuracy", int(4)},
                    {"staggeredMode", int(0)},
                    {"index", int(0)},
                    {"texelDimensions3D", tex_d3d},
                    {"texelDimensions2D", tex_d2d},
                    {"dr", dr},
                    {"dimensions3D", params.simulationDimensions3D}

                }
            );
            this->m_frames.render_tmp.clear();
            this->m_frames.render.clear();
            if (params.useCones) {
                m_conical_arrows3d.view(
                    this->m_frames.render, this->m_frames.tmp,
                    2.0*scale, rotation,
                    params.arrowDimensions,
                    params.dataTexelDimensions3D,
                    {
                        {"useOrthogonalProjection", 
                                (params.usePerspectiveProjection)? int(0): int(1)},
                        {"rescaleZ", int(0)}
                    });
            } else {
                m_arrows3d.view(
                    this->m_frames.render, this->m_frames.tmp,
                    2.0*scale, rotation, 
                    params.arrowDimensions,
                    params.dataTexelDimensions3D,
                    {
                        {"useOrthogonalProjection", 
                                (params.usePerspectiveProjection)? int(0): int(1)},
                        {"rescaleZ", int(0)}
                    });
            }
            WireFrame cube_outline = get_cube_outline_wire_frame();
            this->m_frames.render.draw(
                m_programs.cube_outline,
                {
                    {"rotation", rotation},
                    {"viewScale", scale},
                    {"color", Vec4{.ind{1.0, 1.0, 1.0, 0.5}}},
                    {"usePerspectiveProjection",
                        (params.usePerspectiveProjection)? int(1): int(0)},
                    {"screenDimensions", m_frames.render.texture_dimensions()}
                },
                cube_outline
            );
            WireFrame axes = axes3d::get_axes_wireframe();
            WireFrame axes_labels = axes3d::get_xyz_axes_labels_wireframe();
            axes3d::draw_axes(
                this->m_frames.render,
                {.axes=m_programs.axes_3d, .labels=m_programs.axes_labels_3d},
                axes, axes_labels,
                rotation, 110, 0.0F,
                params.usePerspectiveProjection, 
                m_frames.render.texture_dimensions());
            if (hover.has_value()) {
                IVec2 tex_dims = m_frames.render.texture_dimensions();
                Vec3 r = Vec3{
                    .x=hover->x,
                    .y=hover->y*(float(tex_dims[1])/float(tex_dims[0]))
                        + 0.5F*(1.0F - float(tex_dims[1])/float(tex_dims[0])),
                    .z=0.0};
                r = 2.0*scale_rotate(r, scale, rotation);
                if (r.x >= -1.0 && r.x < 1.0 && 
                    r.y >= -1.0 && r.y < 1.0 &&
                    r.z >= -1.0 && r.z < 1.0) {
                    this->m_cursor_location = r;
                    std::cout << r.x << ", " << r.y << ", " << r.z << std::endl;
                    WireFrame cursor_frame 
                        = cursor_outline3d::get_cursor_wire_frame();
                    this->m_frames.render.draw(
                        m_programs.cursor_outline,
                        {
                            {"rotation", rotation},
                            {"viewScale", scale},
                            {"cursorPosition", r},
                            {"color", Vec4{.ind{0.3, 0.3, 0.3, 0.1}}},
                            {"usePerspectiveProjection",
                                (params.usePerspectiveProjection)? int(1): int(0)}
                        },
                        cursor_frame
                    );
                }
            }
            take_screenshot(
                params, m_frames.render, 
                m_image_data, m_image_rgba_arr);
            return this->m_frames.render;
        }
        case VOL_RENDER_VIEW: case VOL_RENDER_VECTOR_FIELD_VIEW: {
            this->m_frames.render.clear();
            this->m_frames.render_tmp.clear();
            // this->m_frames.render_tmp2.clear();
            WireFrame cube_outline = get_cube_outline_wire_frame();
            m_volume_render.view(
                this->m_frames.render, this->m_frames.data,
                scale, rotation,
                params.alphaBrightness, 
                params.colorBrightness,
                params.usePerspectiveProjection
                // {{"noiseScale", params.noiseScale}}
            );
            if (params.blurSize >= 1 && params.applyBlur) { 
                this->m_frames.render_tmp.draw(
                    m_programs.blur,
                    {{"tex", {this->m_frames.render}}, 
                    {"textureDimensions2D",
                            m_frames.render.texture_dimensions()},
                    {"orientation", int(0)},
                    {"size", int(params.blurSize)}},
                    m_frames.quad_wire_frame
                );
                this->m_frames.render.draw(
                    m_programs.blur,
                    {{"tex", {this->m_frames.render_tmp}}, 
                    {"textureDimensions2D",
                        m_frames.render.texture_dimensions()},
                    {"orientation", int(1)},
                    {"size", int(params.blurSize)}},
                    m_frames.quad_wire_frame
                );
            }
            // this->m_frames.render.draw(
            //     m_programs.copy,
            //     {{"tex", {this->m_frames.render_tmp2}}},
            //     m_frames.quad_wire_frame
            // );
            if (params.visualizationSelect.selected
                    == VOL_RENDER_VECTOR_FIELD_VIEW) {
                IVec3 arrows_d3d = params.arrowDimensions;
                IVec2 arrows_d2d = get_2d_from_3d_dimensions(arrows_d3d);
                IVec3 tex_d3d = params.dataTexelDimensions3D;
                IVec2 tex_d2d = get_2d_from_3d_dimensions(tex_d3d);
                Vec3 dr = Vec3{
                    .x=params.simulationDimensions3D.x/tex_d3d.x,
                    .y=params.simulationDimensions3D.y/tex_d3d.y,
                    .z=params.simulationDimensions3D.z/tex_d3d.z};
                this->m_frames.tmp.draw(
                    m_programs.gradient,
                    {
                        {"tex", &m_frames.data},
                        {"orderOfAccuracy", int(4)},
                        {"staggeredMode", int(0)},
                        {"index", int(0)},
                        {"texelDimensions3D", tex_d3d},
                        {"texelDimensions2D", tex_d2d},
                        {"dr", dr},
                        {"dimensions3D", params.simulationDimensions3D}

                    }
                );
                if (params.useCones) {
                    m_conical_arrows3d.view(
                        this->m_frames.render, this->m_frames.tmp,
                        2.0*scale, rotation, 
                        params.arrowDimensions,
                        params.dataTexelDimensions3D,
                        {
                            {"useOrthogonalProjection",
                                (params.usePerspectiveProjection)? int(0): int(1)},
                            {"rescaleZ", int(1)}
                        });
                } else {
                    m_arrows3d.view(
                        this->m_frames.render, this->m_frames.tmp,
                        2.0*scale, rotation, 
                        params.arrowDimensions,
                        params.dataTexelDimensions3D,
                        {
                            {"useOrthogonalProjection",
                                (params.usePerspectiveProjection)? int(0): int(1)},
                            {"rescaleZ", int(1)}
                        });
                }
            }
            this->m_frames.render.draw(
                m_programs.cube_outline,
                {
                    {"rotation", rotation},
                    {"viewScale", scale},
                    {"color", Vec4{.ind{1.0, 1.0, 1.0, 0.5}}},
                    {"usePerspectiveProjection", 
                            int(params.usePerspectiveProjection)},
                    {"screenDimensions", m_frames.render.texture_dimensions()}
                },
                cube_outline
            );
            if (hover.has_value()) {
                IVec2 tex_dims = m_frames.render.texture_dimensions();
                Vec3 r = Vec3{
                    .x=hover->x,
                    .y=hover->y*(float(tex_dims[1])/float(tex_dims[0]))
                        + 0.5F*(1.0F - float(tex_dims[1])/float(tex_dims[0])),
                    .z=0.0};
                r = 2.0*scale_rotate(r, scale, rotation);
                if (r.x >= -1.0 && r.x < 1.0 && 
                    r.y >= -1.0 && r.y < 1.0 &&
                    r.z >= -1.0 && r.z < 1.0) {
                    this->m_cursor_location = r;
                    // std::cout << r.x << ", " << r.y << ", " << r.z << std::endl;
                    WireFrame cursor_frame 
                        = cursor_outline3d::get_cursor_wire_frame();
                    this->m_frames.render.draw(
                        m_programs.cursor_outline,
                        {
                            {"rotation", rotation},
                            {"viewScale", scale},
                            {"cursorPosition", r},
                            {"color", Vec4{.ind{0.3, 0.3, 0.3, 0.1}}},
                            {"usePerspectiveProjection", 
                                    int(params.usePerspectiveProjection)},
                            {"screenDimensions", m_frames.render.texture_dimensions()}
                        },
                        cursor_frame
                    );
                }
            }
            WireFrame axes = axes3d::get_axes_wireframe();
            WireFrame axes_labels = axes3d::get_xyz_axes_labels_wireframe();
            axes3d::draw_axes(
                this->m_frames.render,
                {.axes=m_programs.axes_3d, .labels=m_programs.axes_labels_3d},
                axes, axes_labels,
                rotation, 110, 0.0F,
                params.usePerspectiveProjection, 
                m_frames.render.texture_dimensions());
            take_screenshot(
                params, m_frames.render, 
                m_image_data, m_image_rgba_arr);
            return this->m_frames.render;
        }
    }
}

const RenderTarget &Simulation
::view_data_texture(SimParams &params, ::Quaternion rotation, float scale) {
    m_frames.render.draw(
        m_programs.copy,
        {{"tex", &m_frames.data}},
        m_frames.quad_wire_frame
    );
    return m_frames.render;
}

const RenderTarget &Simulation
::view_volume_texture(
    SimParams &params, ::Quaternion rotation, float scale
    ) {
    m_frames.render.draw(
        m_programs.copy,
        {{"tex", &m_frames.data}},
        m_frames.quad_wire_frame
    );
    return m_frames.render;
}

void Simulation::add_user_defined(
    const SimParams &sim_params,
    unsigned int program, const std::map<std::string, float> &input_uniforms) {
    this->m_programs.user_defined = program;
    Uniforms uniforms;
    for (const auto &e: input_uniforms)
        uniforms.insert({e.first, Vec2{.ind{e.second, 0.0}}});
    if (input_uniforms.count("t") > 0)
        uniforms.at("t").vec2.x = sim_params.t;
    else
        uniforms.insert({"t", Vec2{.ind{sim_params.t, 0.0}}});
    uniforms.insert(
        {"width",
            Vec2{.ind{sim_params.simulationDimensions3D[0], 0.0}}});
    uniforms.insert(
        {"height",
            Vec2{.ind{sim_params.simulationDimensions3D[1], 0.0}}});
    uniforms.insert(
        {"depth",
            Vec2{.ind{sim_params.simulationDimensions3D[2], 0.0}}});
    uniforms.insert(
        {
            "texelDimensions2D",
            get_2d_from_3d_dimensions(sim_params.dataTexelDimensions3D)
        }
    );
    uniforms.insert(
        {
            "texelDimensions3D",
            sim_params.dataTexelDimensions3D
        }
    );
    enum outputModeSelect {
        MODE_4VECTOR_REAL_OR_COMPLEX=0,
        MODE_COMPLEX4=4
    };
    uniforms.insert(
        {"outputModeSelect",int(MODE_COMPLEX4)}
    );
    uniforms.insert(
        {"useRealPartOfExpression", int(1)});
    m_frames.tmp.draw(program, uniforms);
    m_frames.data.draw(
        m_programs.domain_color,
        {
            {"tex", &m_frames.tmp},
            {"brightness", sim_params.brightness},
            {"brightnessMode", int(1)},

        }
    );
}

void Simulation::reset_data_dimensions(IVec3 texel_dimensions_3d) {
    m_volume_render.reset_data_dimensions(texel_dimensions_3d);
    m_frames.reset_data_dimensions(texel_dimensions_3d);
}

void Simulation::reset_volume_dimensions(IVec3 texel_dimensions_3d) {
    m_volume_render.reset_volume_dimensions(texel_dimensions_3d);
}

void Simulation::reset_volume_filtering(unsigned int filtering) {
    m_volume_render.reset_filtering(filtering);
}

Vec3 Simulation::get_cursor_location() {
    return m_cursor_location;
}

Vec3 Simulation::get_scaled_cursor_location(const SimParams &params) {
    return Vec3{
        .x=m_cursor_location.x*params.simulationDimensions3D.x/2.0F,
        .y=m_cursor_location.y*params.simulationDimensions3D.y/2.0F,
        .z=m_cursor_location.z*params.simulationDimensions3D.z/2.0F,
    };
}

std::vector<unsigned char> &Simulation::get_image_data() {
    return m_image_data;
}

