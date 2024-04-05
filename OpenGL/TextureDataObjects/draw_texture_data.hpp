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

class Drawer;

class Uniform {
    enum {
        BOOL,
        FLOAT, FLOAT2, FLOAT3, FLOAT4,
        INT, INT2, INT3, INT4,
        DOUBLE, DOUBLE2,
        TEX,
    };
    int type;
    union {
        union {
            union {
                int i32;
                float f32;
                int b32;
            };
            struct IVec2 ivec2;
            struct Vec2 vec2;
            double f64;
            const Texture2DData *tex;
        };
        Vec3 vec3;
        IVec3 ivec3;
        Vec4 vec4;
        IVec4 ivec4;
    };
    friend Drawer;
    public:
    Uniform(bool b):b32(b) {type=Uniform::BOOL;}
    Uniform(int i):i32(i) {type=Uniform::INT;}
    Uniform(float f):f32(f) {type=Uniform::FLOAT;}
    Uniform(double d):f64(d) {type=Uniform::DOUBLE;}
    Uniform(struct Vec2 v):vec2(v) {type=Uniform::FLOAT2;}
    Uniform(struct IVec2 v):ivec2(v) {type=Uniform::INT2;}
    Uniform(struct Vec3 v):vec3(v) {type=Uniform::FLOAT3;}
    Uniform(struct IVec3 v):ivec3(v) {type=Uniform::INT3;}
    Uniform(struct Vec4 v):vec4(v) {type=Uniform::FLOAT4;}
    Uniform(struct IVec4 v):ivec4(v) {type=Uniform::INT4;}
    Uniform(const Texture2DData *t):tex(t) {type=Uniform::TEX;}
};


class Drawer {
    int program = 0;
    public:
    Drawer(const struct Path &path);
    void draw(const Texture2DData &dst, 
              std::map<std::string, const Uniform &>) const;
};

class DrawTexture2DData {
    int program = 0;
    std::map<std::string, float> float_uniforms {};
    std::map<std::string, int> int_uniforms{};
    std::map<std::string, struct Vec2> vec2_uniforms {};
    std::map<std::string, struct Vec3> vec3_uniforms {};
    std::map<std::string, struct Vec4> vec4_uniforms {};
    std::map<std::string, struct IVec2> ivec2_uniforms {};
    std::map<std::string, struct IVec3> ivec3_uniforms {};
    void use_uniforms() const;
public:
    DrawTexture2DData(int program);
    DrawTexture2DData(std::string &fragment_src);
    DrawTexture2DData(const struct Path &path);
    void set_float_uniform(std::string name, float val);
    void set_float_uniforms(std::map<std::string, float> float_uniforms);
    void set_int_uniform(std::string name, int val);
    void set_int_uniforms(std::map<std::string, int> int_uniforms);
    void set_vec2_uniform(std::string name, struct Vec2 val);
    void set_vec2_uniforms(std::map<std::string, struct Vec2> vec2_uniforms);
    void set_vec3_uniform(std::string name, struct Vec3 val);
    void set_vec3_uniforms(std::map<std::string, struct Vec3> vec3_uniforms);
    void set_vec4_uniform(std::string name, struct Vec4 val);
    void set_vec4_uniforms(std::map<std::string, struct Vec4> vec4_uniforms);
    void set_ivec2_uniform(std::string name, struct IVec2 val);
    void set_ivec2_uniforms(std::map<std::string, struct IVec2>
                            ivec2_uniforms);
    void set_ivec3_uniform(std::string name, struct IVec3 val);
    void set_ivec3_uniforms(std::map<std::string, struct IVec3>
                            ivec2_uniforms);
    void draw(const Texture2DData &tex2d_data) const;
    Texture2DData create(int type, int width, int height,
                         bool generate_mipmap=true ,
                         GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                         GLuint min_filter=GL_LINEAR,
                         GLuint mag_filter=GL_LINEAR);
    void draw(const Texture2DData &tex2d_data,
              std::string str0, const Texture2DData &x0) const;
    void draw(const Texture2DData &tex2d_data,
              std::string str0, const Texture2DData &x0,
              std::string str1, const Texture2DData &x1) const;
    void draw(const Texture2DData &tex2d_data,
              std::string str0, const Texture2DData &x0,
              std::string str1, const Texture2DData &x1,
              std::string str2, const Texture2DData &x2) const;
    void draw(const Texture2DData &tex2d_data,
              std::string str0, const Texture2DData &x0,
              std::string str1, const Texture2DData &x1,
              std::string str2, const Texture2DData &x2,
              std::string str3, const Texture2DData &x3) const;
};

#endif

