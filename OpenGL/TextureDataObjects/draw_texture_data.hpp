#include <map>
#include <string>
#include "texture_data.hpp"

#ifndef _DRAW_TEXTURE_DATA_
#define _DRAW_TEXTURE_DATA_


struct Path {
    std::string path;
    Path(std::string path) {
        this->path = path;
    }
};

class DrawTexture2DData {
    int program = 0;
    std::map<std::string, float> float_uniforms {};
    std::map<std::string, int> int_uniforms{};
    std::map<std::string, int> sampler2D_uniforms {};
    std::map<std::string, struct Vec2> vec2_uniforms {};
    std::map<std::string, struct Vec3> vec3_uniforms {};
    std::map<std::string, struct Vec4> vec4_uniforms {};
    std::map<std::string, struct IVec2> ivec2_uniforms {};
    std::map<std::string, struct IVec3> ivec3_uniforms {};
public:
    DrawTexture2DData(int program);
    DrawTexture2DData(std::string &fragment_src);
    DrawTexture2DData(const struct Path &path);
    void set_texture2D_data(std::string &name,
                            const Texture2DData &tex2d_data);
    void set_float_uniforms(std::map<std::string, float> float_uniforms);
    void set_int_uniforms(std::map<std::string, int> int_uniforms);
    void set_vec2_uniforms(std::map<std::string, struct Vec2> vec2_uniforms);
    void set_vec3_uniforms(std::map<std::string, struct Vec3> vec3_uniforms);
    void set_vec4_uniforms(std::map<std::string, struct Vec4> vec4_uniforms);
    void set_ivec2_uniforms(std::map<std::string, struct IVec2>
                            ivec2_uniforms);
    void set_ivec3_uniforms(std::map<std::string, struct IVec3>
                            ivec2_uniforms);
    void draw(const Texture2DData &tex2d_data);
};

#endif

