#include "draw_texture_data.hpp"


Drawer::Drawer(const struct Path &path) {
    this->program = make_quad_program(path.path.c_str());
}

void Drawer::draw(const Texture2DData &dst,
                  std::map<std::string, const Uniform &> uniforms) const {
    bind_quad(dst.get_frame_id(), this->program);
    for (auto &e: uniforms) {
        switch(e.second.type) {
            case Uniform::BOOL:
            set_int_uniform((char *)&e.first[0], e.second.b32);
            case Uniform::INT:
            set_int_uniform((char *)&e.first[0], e.second.i32);
            break;
            case Uniform::INT2:
            set_ivec2_uniform(
                (char *)&e.first[0],
                e.second.ivec2.x, e.second.ivec2.y);
            break;
            case Uniform::INT3:
            set_ivec3_uniform(
                (char *)&e.first[0], 
                e.second.ivec3.x, e.second.ivec3.y, e.second.ivec3.z);
            break;
            /* case Uniform::INT4:
            set_ivec4_uniform(
                (char *)&e.first[0], 
                e.second.ivec4.x, e.second.ivec4.y,
                e.second.ivec4.z, e.second.ivec4.w);
            break;*/
            case Uniform::FLOAT:
            set_float_uniform((char *)&e.first[0], e.second.f32);
            break;
            case Uniform::FLOAT2:
            set_vec2_uniform(
                (char *)&e.first[0], e.second.vec2.x, e.second.vec2.y);
            break;
            case Uniform::FLOAT3:
            set_vec3_uniform(
                (char *)&e.first[0], 
                e.second.vec3.x, e.second.vec3.y, e.second.vec3.z);
            break;
            case Uniform::FLOAT4:
            set_vec4_uniform(
                (char *)&e.first[0], 
                e.second.vec4.x, e.second.vec4.y, 
                e.second.vec4.z, e.second.vec4.w);
            break;
            /* case Uniform::DOUBLE:
            set_int_uniform((char *)&e.first[0], e.second.i32);
            break;
            case Uniform::DOUBLE2:
            set_int_uniform((char *)&e.first[0], e.second.i32);
            break;*/
            case Uniform::TEX:
            set_sampler2D_uniform(
                (char *)&e.first[0], e.second.tex->get_frame_id());
            break;
        }
    }
    draw_unbind_quad();
}

DrawTexture2DData::DrawTexture2DData(int program) {
    this->program = program;
}

DrawTexture2DData::DrawTexture2DData(std::string &fragment_src) {
    this->program
        = make_quad_program_from_string_source(fragment_src.c_str());
}

DrawTexture2DData::DrawTexture2DData(const struct Path &path) {
    this->program = make_quad_program(path.path.c_str());
}

/*void DrawTexture2DData::set_texture2D_data(const std::string &name,
                        const Texture2DData &tex2d_data) {
    this->sampler2D_uniforms[name] = tex2d_data.frame;
}

void DrawTexture2DData::set_texture2D_data(std::string &&name,
                        const Texture2DData &tex2d_data) {
    this->sampler2D_uniforms[name] = tex2d_data.frame;
}*/

void DrawTexture2DData::set_float_uniform(std::string k, float val) {
    this->float_uniforms[k] = val;
}

void
DrawTexture2DData::set_float_uniforms(std::map<std::string, float>
                                      float_uniforms) {
    for (auto &e: float_uniforms)
        this->float_uniforms[e.first] = e.second;
}

void DrawTexture2DData::set_int_uniform(std::string k, int val) {
    this->int_uniforms[k] = val;
}

void
DrawTexture2DData::set_int_uniforms(std::map<std::string, int>
                                    int_uniforms) {
    for (auto &e: int_uniforms)
        this->float_uniforms[e.first] = e.second;
}

void DrawTexture2DData::set_vec2_uniform(std::string k, struct Vec2 val) {
    this->vec2_uniforms[k] = val;
}

void
DrawTexture2DData::set_vec2_uniforms(std::map<std::string, struct Vec2>
                                     vec2_uniforms) {
    for (auto &e: vec2_uniforms)
        this->vec2_uniforms[e.first] = e.second;
}

void DrawTexture2DData::set_vec3_uniform(std::string k, struct Vec3 val) {
    this->vec3_uniforms[k] = val;
}

void DrawTexture2DData::set_vec3_uniforms(std::map<std::string, struct Vec3>
                                          vec3_uniforms) {
    for (auto &e: vec3_uniforms)
        this->vec3_uniforms[e.first] = e.second;
}

void DrawTexture2DData::set_vec4_uniform(std::string k, struct Vec4 val) {
    this->vec4_uniforms[k] = val;
}

void DrawTexture2DData::set_vec4_uniforms(std::map<std::string, struct Vec4>
                                          vec4_uniforms) {
    for (auto &e: vec4_uniforms)
        this->vec4_uniforms[e.first] = e.second;
}

void DrawTexture2DData::set_ivec2_uniform(std::string name, struct IVec2 val) {
    this->ivec2_uniforms[name] = val;
}

void DrawTexture2DData::set_ivec2_uniforms(std::map<std::string, struct IVec2>
                        ivec2_uniforms) {
    for (auto &e: ivec2_uniforms)
        this->ivec2_uniforms[e.first] = e.second;
}

void DrawTexture2DData::set_ivec3_uniform(std::string name, struct IVec3 val) {
    this->ivec3_uniforms[name] = val;
}

void DrawTexture2DData::set_ivec3_uniforms(std::map<std::string, struct IVec3>
                        ivec3_uniforms) {
    for (auto &e: ivec3_uniforms)
        this->ivec3_uniforms[e.first] = e.second;
}

void DrawTexture2DData::use_uniforms() const {
    for (auto &e: float_uniforms)
        ::set_float_uniform(e.first.c_str(), e.second);
    for (auto &e: int_uniforms)
        ::set_int_uniform(e.first.c_str(), e.second);
    for (auto &e: vec2_uniforms)
        ::set_vec2_uniform(e.first.c_str(), e.second.x, e.second.y);
    for (auto &e: vec3_uniforms)
        ::set_vec3_uniform(e.first.c_str(),
                           e.second.x, e.second.y, e.second.z);
    for (auto &e: vec4_uniforms)
        ::set_vec4_uniform(e.first.c_str(),
                            e.second.x, e.second.y, e.second.z, e.second.w);
    for (auto &e: ivec2_uniforms)
        ::set_ivec2_uniform(e.first.c_str(),
                            e.second.x, e.second.y);
    for (auto &e: ivec3_uniforms)
        ::set_ivec3_uniform(e.first.c_str(),
                            e.second.x, e.second.y, e.second.z);
}

void DrawTexture2DData::draw(const Texture2DData &tex2d_data) const {
    bind_quad(tex2d_data.frame, this->program);
    use_uniforms();
    draw_unbind_quad();
}

void DrawTexture2DData::draw(const Texture2DData &tex2d_data,
                             std::string str0,
                             const Texture2DData &x0) const {
    if (tex2d_data.frame == x0.frame) {
        auto tmp = tex2d_data;
        bind_quad(tex2d_data.frame, this->program);
        use_uniforms();
        ::set_sampler2D_uniform(str0.c_str(), tmp.frame);
        draw_unbind_quad();
        return;
    }
    bind_quad(tex2d_data.frame, this->program);
    use_uniforms();
    ::set_sampler2D_uniform(str0.c_str(), x0.frame);
    draw_unbind_quad();
}

void DrawTexture2DData::draw(const Texture2DData &tex2d_data,
                             std::string str0, const Texture2DData &x0,
                             std::string str1, const Texture2DData &x1
                             ) const {
    if (tex2d_data.frame == x0.frame || tex2d_data.frame == x1.frame) {
        auto tmp0 = (tex2d_data.frame == x0.frame)? tex2d_data: x0;
        auto tmp1 = (tex2d_data.frame == x1.frame)? tex2d_data: x1;
        bind_quad(tex2d_data.frame, this->program);
        use_uniforms();
        ::set_sampler2D_uniform(str0.c_str(), tmp0.frame);
        ::set_sampler2D_uniform(str1.c_str(), tmp1.frame);
        draw_unbind_quad();
        return;
    }
    bind_quad(tex2d_data.frame, this->program);
    use_uniforms();
    ::set_sampler2D_uniform(str0.c_str(), x0.frame);
    ::set_sampler2D_uniform(str1.c_str(), x1.frame);
    draw_unbind_quad();
}

void DrawTexture2DData::draw(const Texture2DData &tex2d_data,
                             std::string str0, const Texture2DData &x0,
                             std::string str1, const Texture2DData &x1,
                             std::string str2, const Texture2DData &x2
                            ) const {
    if (tex2d_data.frame == x0.frame || tex2d_data.frame == x1.frame
        || tex2d_data.frame == x2.frame) {
        auto tmp0 = (tex2d_data.frame == x0.frame)? tex2d_data: x0;
        auto tmp1 = (tex2d_data.frame == x1.frame)? tex2d_data: x1;
        auto tmp2 = (tex2d_data.frame == x2.frame)? tex2d_data: x2;
        bind_quad(tex2d_data.frame, this->program);
        use_uniforms();
        ::set_sampler2D_uniform(str0.c_str(), tmp0.frame);
        ::set_sampler2D_uniform(str1.c_str(), tmp1.frame);
        ::set_sampler2D_uniform(str2.c_str(), tmp2.frame);
        draw_unbind_quad();
        return;
    }
    bind_quad(tex2d_data.frame, this->program);
    use_uniforms();
    ::set_sampler2D_uniform(str0.c_str(), x0.frame);
    ::set_sampler2D_uniform(str1.c_str(), x1.frame);
    ::set_sampler2D_uniform(str2.c_str(), x2.frame);
    draw_unbind_quad();
}

void DrawTexture2DData::draw(const Texture2DData &tex2d_data,
                             std::string str0, const Texture2DData &x0,
                             std::string str1, const Texture2DData &x1,
                             std::string str2, const Texture2DData &x2,
                             std::string str3, const Texture2DData &x3
                            ) const {
    if (tex2d_data.frame == x0.frame || tex2d_data.frame == x1.frame
        || tex2d_data.frame == x2.frame) {
        auto tmp0 = (tex2d_data.frame == x0.frame)? tex2d_data: x0;
        auto tmp1 = (tex2d_data.frame == x1.frame)? tex2d_data: x1;
        auto tmp2 = (tex2d_data.frame == x2.frame)? tex2d_data: x2;
        auto tmp3 = (tex2d_data.frame == x3.frame)? tex2d_data: x3;
        bind_quad(tex2d_data.frame, this->program);
        use_uniforms();
        ::set_sampler2D_uniform(str0.c_str(), tmp0.frame);
        ::set_sampler2D_uniform(str1.c_str(), tmp1.frame);
        ::set_sampler2D_uniform(str2.c_str(), tmp2.frame);
        ::set_sampler2D_uniform(str3.c_str(), tmp3.frame);
        draw_unbind_quad();
        return;
    }
    bind_quad(tex2d_data.frame, this->program);
    use_uniforms();
    ::set_sampler2D_uniform(str0.c_str(), x0.frame);
    ::set_sampler2D_uniform(str1.c_str(), x1.frame);
    ::set_sampler2D_uniform(str2.c_str(), x2.frame);
    ::set_sampler2D_uniform(str3.c_str(), x3.frame);
    draw_unbind_quad();
}

Texture2DData DrawTexture2DData::create(int type, int width, int height,
                                        bool generate_mipmap,
                                        GLuint wrap_s, GLuint wrap_t,
                                        GLuint min_filter, GLuint mag_filter) {
    Texture2DData x = Texture2DData(type, width, height,
                                    generate_mipmap, wrap_s, wrap_t,
                                    min_filter, mag_filter);
    this->draw(x);
    return x;
}
