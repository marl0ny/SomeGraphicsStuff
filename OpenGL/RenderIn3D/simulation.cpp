#include "simulation.hpp"

using namespace sim_2d;


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
        .generate_mipmap=1, // default_tex_params.generate_mipmap,
        .min_filter=default_tex_params.min_filter,
        .mag_filter=default_tex_params.mag_filter,
        .wrap_s=GL_REPEAT,
        .wrap_t=GL_REPEAT
    }),
    data(data_tex_params),
    tmp(data_tex_params),
    render(default_tex_params),
    quad_wire_frame(get_quad_wire_frame()),
    arrows3d_frame(
        arrows3d::get_3d_vector_field_wire_frame(
            params.arrowDimensions)) {
}

void Frames::reset_data_dimensions(IVec3 texel_dimensions3d) {
    IVec2 texel_dimensions2d = get_2d_from_3d_dimensions(texel_dimensions3d);
    this->data_tex_params.width = texel_dimensions2d[0];
    this->data_tex_params.height = texel_dimensions2d[1];
    data.reset(this->data_tex_params);
    tmp.reset(this->data_tex_params);
}

Simulation::
Simulation(const TextureParams &default_tex_params, const SimParams &params
) : m_volume_render(default_tex_params,
    params.volumeTexelDimensions3D,
    params.dataTexelDimensions3D),
    m_planar_slices(default_tex_params),
    m_arrows3d(params.arrowDimensions, default_tex_params),
    m_frames(default_tex_params, params) {
}

const RenderTarget &Simulation
::view(SimParams &params, ::Quaternion rotation, float scale) {
    enum {VOL_RENDER_VIEW=0, PLANAR_SLICES_VIEW=1, VECTOR_FIELD_VIEW=2};
    switch(params.visualizationSelect.selected) {
        case PLANAR_SLICES_VIEW:
        return m_planar_slices.view(
            this->m_frames.data, params.dataTexelDimensions3D, 
            rotation, scale,
            int(params.dataTexelDimensions3D.z
                *params.planarNormCoordOffsets[0]),
            int(params.dataTexelDimensions3D.x
                *params.planarNormCoordOffsets[1]),
            int(params.dataTexelDimensions3D.y
                *params.planarNormCoordOffsets[2]),
            Vec2{.ind {0.0, 0.0}}
        );
        case VECTOR_FIELD_VIEW:
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
            this->m_frames.render.clear();
            m_arrows3d.view(
                this->m_frames.render, this->m_frames.tmp,
                scale, rotation, 
                params.arrowDimensions,
                params.dataTexelDimensions3D);
            return this->m_frames.render;
        }
        case VOL_RENDER_VIEW:
        return m_volume_render.view(
            this->m_frames.data, scale, rotation,
            params.alphaBrightness, params.colorBrightness,
            {{"noiseScale", params.noiseScale}}
        );
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
