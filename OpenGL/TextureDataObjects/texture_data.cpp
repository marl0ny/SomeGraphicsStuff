#include "bin_ops.h"
#include "unary_ops.h"
#include "frames_stacks.h"
#include <GLES3/gl3.h>
#include <cmath>
#include <complex>
#include <iostream>
#include <map>
#include <string>
#include <vector>
// #include <vector>
#include <GLFW/glfw3.h>
#include "texture_data.hpp"


static int type_to_format(GLuint format) {
    switch(format) {
        case FLOAT:
        return GL_R32F;
        case FLOAT2: case COMPLEX:
        return GL_RG32F;
        case FLOAT3:
        return GL_RGB32F;
        case FLOAT4: case COMPLEX2:
        return GL_RGBA32F;
        case HALF_FLOAT:
        return GL_R16F;
        case HALF_FLOAT2:
        return GL_RG16F;
        case HALF_FLOAT3:
        return GL_RGB16F;
        case HALF_FLOAT4:
        return GL_RGBA16F;
        case INT:
        return GL_R32I;
        case INT2:
        return GL_RG32I;
        case INT3:
        return GL_RGB32I;
        case INT4:
        return GL_RGBA32I;
        case UINT:
        return GL_R32UI;
        case UINT2:
        return GL_RG32UI;
        case UINT3:
        return GL_RGB32UI;
        case UINT4:
        return GL_RGBA32UI;
        case SHORT:
        return GL_R16I;
        case SHORT2:
        return GL_RG16I;
        case SHORT3:
        return GL_RGB16I;
        case SHORT4:
        return GL_RGBA16I;
        case USHORT:
        return GL_R16UI;
        case USHORT2:
        return GL_RG16UI;
        case USHORT3:
        return GL_RGB16UI;
        case USHORT4:
        return GL_RGBA16UI;
        case BYTE:
        return GL_R8;
        case BYTE2:
        return GL_RG8;
        case BYTE3:
        return GL_RGB8;
        case BYTE4:
        return GL_RGBA8;
        case UBYTE:
        return GL_R8UI;
        case UBYTE2:
        return GL_RG8UI;
        case UBYTE3:
        return GL_RGB8UI;
        case UBYTE4:
        return GL_RGBA8UI;

    }
    return -1;
}

Texture2DData::Texture2DData(int type, frame_id frame,
                             const struct TextureParams &tex_params) {
    this->type = type;
    this->frame = frame;
    this->tex_params = tex_params;
}

Texture2DData::Texture2DData(int type, int width, int height,
                  bool generate_mipmap,
                  GLuint wrap_s, GLuint wrap_t,
                  GLuint min_filter, GLuint mag_filter) {
    this->type = type;
    this->tex_params = {
        .format=type_to_format(type),
        .width=width, .height=height,
        .generate_mipmap = (int)generate_mipmap,
        .wrap_s = (int)wrap_s, .wrap_t = (int)wrap_t,
        .min_filter = (int)min_filter, .mag_filter = (int)mag_filter,
    };
    frame = activate_frame(&tex_params);
}

Texture2DData::Texture2DData(const Texture2DData &x) {
    this->type = x.type;
    this->tex_params = {
        .format=x.tex_params.format,
        .width=x.tex_params.width, .height=x.tex_params.height,
        .generate_mipmap = x.tex_params.generate_mipmap,
        .wrap_s = x.tex_params.wrap_s, .wrap_t = x.tex_params.wrap_t,
        .min_filter = x.tex_params.min_filter,
        .mag_filter = x.tex_params.mag_filter,
    };
    frame = activate_frame(&tex_params);
    tex_copy(this->frame, x.frame);
}

Texture2DData& Texture2DData::operator=(const Texture2DData &x) {
    std::cout << "copy assignment called." << std::endl;
    tex_copy(this->frame, x.frame);
    return *this;
}

Texture2DData::Texture2DData(Texture2DData &&x) {
    // Constructor so frame should be uninitialized.
    this->frame = x.frame;
    this->type = x.type;
    this->tex_params = {
        .format=x.tex_params.format,
        .width=x.tex_params.width, .height=x.tex_params.height,
        .generate_mipmap = x.tex_params.generate_mipmap,
        .wrap_s = x.tex_params.wrap_s, .wrap_t = x.tex_params.wrap_t,
        .min_filter = x.tex_params.min_filter,
        .mag_filter = x.tex_params.mag_filter,
    };
    x.frame = -1;
}

Texture2DData& Texture2DData::operator=(Texture2DData &&x) {
    deactivate_frame(&this->tex_params, this->frame);
    this->frame = x.frame;
    this->type = x.type;
    this->tex_params = {
        .format=x.tex_params.format,
        .width=x.tex_params.width, .height=x.tex_params.height,
        .generate_mipmap = x.tex_params.generate_mipmap,
        .wrap_s = x.tex_params.wrap_s, .wrap_t = x.tex_params.wrap_t,
        .min_filter = x.tex_params.min_filter,
        .mag_filter = x.tex_params.mag_filter,
    };
    x.frame = -1;
    return *this;
}

void Texture2DData::paste_to_quad(frame_id quad) {
    tex_copy(quad, this->frame);
}

void Texture2DData::set_as_sampler2D_uniform(const char *name) {
    set_sampler2D_uniform(name, this->frame);
}

void Texture2DData::debug_print_state() {
    std::cout << "type_id: " << type << "\n";
    std::cout << "frame_id: " << frame << "\n";
}

Texture2DData Texture2DData::cast_to(int type, Channel c0) {
    struct TextureParams tex_params {
        .format = type_to_format(type),
        .width = this->tex_params.width, .height = this->tex_params.height,
        .generate_mipmap = this->tex_params.generate_mipmap,
        .wrap_s = this->tex_params.wrap_s, .wrap_t = this->tex_params.wrap_t,
        .min_filter = this->tex_params.min_filter,
        .mag_filter = this->tex_params.mag_filter,
    };
    frame_id new_frame = activate_frame(&tex_params);
    tex_swizzle(new_frame, this->frame, (int)c0, -1, -1, -1);
    return Texture2DData(type, new_frame, tex_params);
}

Texture2DData Texture2DData::cast_to(int type, Channel c0, Channel c1) {
    struct TextureParams tex_params {
        .format = type_to_format(type),
        .width = this->tex_params.width, .height = this->tex_params.height,
        .generate_mipmap = this->tex_params.generate_mipmap,
        .wrap_s = this->tex_params.wrap_s, .wrap_t = this->tex_params.wrap_t,
        .min_filter = this->tex_params.min_filter,
        .mag_filter = this->tex_params.mag_filter,
    };
    frame_id new_frame = activate_frame(&tex_params);
    tex_swizzle(new_frame, this->frame, (int)c0, (int)c1, -1, -1);
    return Texture2DData(type, new_frame, tex_params);
}

Texture2DData Texture2DData::cast_to(int type,
                                     Channel c0, Channel c1, Channel c2) {
    struct TextureParams tex_params {
        .format = type_to_format(type),
        .width = this->tex_params.width, .height = this->tex_params.height,
        .generate_mipmap = this->tex_params.generate_mipmap,
        .wrap_s = this->tex_params.wrap_s, .wrap_t = this->tex_params.wrap_t,
        .min_filter = this->tex_params.min_filter,
        .mag_filter = this->tex_params.mag_filter,
    };
    frame_id new_frame = activate_frame(&tex_params);
    tex_swizzle(new_frame, this->frame, (int)c0, (int)c1, (int)c2, -1);
    return Texture2DData(type, new_frame, tex_params);
}

Texture2DData Texture2DData::cast_to(int type,
                                     Channel c0, Channel c1,
                                     Channel c2, Channel c3) {
    struct TextureParams tex_params {
        .format = type_to_format(type),
        .width = this->tex_params.width, .height = this->tex_params.height,
        .generate_mipmap = this->tex_params.generate_mipmap,
        .wrap_s = this->tex_params.wrap_s, .wrap_t = this->tex_params.wrap_t,
        .min_filter = this->tex_params.min_filter,
        .mag_filter = this->tex_params.mag_filter,
    };
    frame_id new_frame = activate_frame(&tex_params);
    tex_swizzle(new_frame, this->frame, (int)c0, (int)c1, (int)c2, (int)c3);
    return Texture2DData(type, new_frame, tex_params);
}


void swap(Texture2DData &x, Texture2DData &y) {
    // Copy members of x into temporary variables
    int tmp_type = x.type; 
    frame_id tmp_frame = x.frame;
    TextureParams tmp_tex_params = {
        .format = x.tex_params.format,
        .generate_mipmap = x.tex_params.generate_mipmap,
        .wrap_s = x.tex_params.wrap_s, .wrap_t = x.tex_params.wrap_t,
        .min_filter = x.tex_params.min_filter,
        .mag_filter = x.tex_params.mag_filter,
    };
    // Copy members of y into x
    x.type = y.type;
    x.frame = y.frame;
    x.tex_params.format = y.tex_params.format;
    x.tex_params.generate_mipmap = y.tex_params.generate_mipmap;
    x.tex_params.wrap_s = y.tex_params.wrap_s;
    x.tex_params.wrap_t = y.tex_params.wrap_t;
    x.tex_params.min_filter = y.tex_params.min_filter;
    x.tex_params.mag_filter = y.tex_params.mag_filter;
    // Copy the temporary variables into y
    y.type = tmp_type;
    y.frame = tmp_frame;
    y.tex_params.format = tmp_tex_params.format;
    y.tex_params.generate_mipmap = tmp_tex_params.generate_mipmap;
    y.tex_params.wrap_s = tmp_tex_params.wrap_s;
    y.tex_params.wrap_t = tmp_tex_params.wrap_t;
    y.tex_params.min_filter = tmp_tex_params.min_filter;
    y.tex_params.mag_filter = tmp_tex_params.mag_filter;

}


static void check_if_types_match(int x_type, int y_type) {
    if (x_type != y_type) {
        std::cerr << "Type mismatch." << std::endl;
    }
}


Texture2DData operator+(const Texture2DData &x, const Texture2DData &y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    check_if_types_match(x.type, y.type);
    tex_tex_add_tex(new_frame, x.frame, y.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator-(const Texture2DData &x, const Texture2DData &y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    check_if_types_match(x.type, y.type);
    tex_tex_sub_tex(new_frame, x.frame, y.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator*(const Texture2DData &x, const Texture2DData &y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    check_if_types_match(x.type, y.type);
    if ((x.type == COMPLEX && y.type == COMPLEX) ||
        (x.type == COMPLEX2 && y.type == COMPLEX2) ||
        (x.type == COMPLEX2 && y.type == COMPLEX) ||
        (x.type == COMPLEX && y.type == COMPLEX2)) {
        tex_tex_complex_mul_tex(new_frame, x.frame, y.frame);
        return Texture2DData(x.type, new_frame, x.tex_params);
    }
    tex_tex_mul_tex(new_frame, x.frame, y.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator+(const Texture2DData &x, struct Vec2 &v) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_tex_add_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator+(struct Vec2 &v, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_vec2_add_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator-(const Texture2DData &x, struct Vec2 &v) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_tex_sub_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator-(struct Vec2 &v, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_vec2_sub_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator*(const Texture2DData &x, struct Vec2 &v) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_tex_mul_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator*(struct Vec2 &v, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_vec2_mul_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator/(const Texture2DData &x, struct Vec2 &v) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_tex_div_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator/(struct Vec2 &v, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_vec2_div_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator+(const Texture2DData &x, std::complex<double> z) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_tex_add_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator+(std::complex<double> z, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_vec2_add_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator-(const Texture2DData &x, std::complex<double> z) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_tex_sub_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator-(std::complex<double> z, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_vec2_sub_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator*(const Texture2DData &x, std::complex<double> z) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_tex_complex_mul_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator*(std::complex<double> z, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_vec2_complex_mul_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator/(const Texture2DData &x, std::complex<double> z) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_tex_complex_div_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator/(std::complex<double> z, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_vec2_complex_div_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator+(const Texture2DData &x, double y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_tex_add_float(new_frame, x.frame, (float)y);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator+(double y, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_float_add_tex(new_frame, (float)y, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator-(const Texture2DData &x, double y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_tex_sub_float(new_frame, x.frame, (float)y);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator-(double y, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_float_sub_tex(new_frame, (float)y, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator*(const Texture2DData &x, double y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_tex_complex_mul_float(new_frame, x.frame, (float)y);
    else
        tex_tex_mul_float(new_frame, x.frame, (float)y);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator*(double y, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_float_complex_mul_tex(new_frame, (float)y, x.frame);
    else
        tex_float_mul_tex(new_frame, (float)y, x.frame);;
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator/(const Texture2DData &x, double y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_tex_complex_div_float(new_frame, x.frame, (float)y);
    else
        tex_tex_div_float(new_frame, x.frame, (float)y);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator/(double y, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_float_complex_div_tex(new_frame, (float)y, x.frame);
    else
        tex_float_div_tex(new_frame, (float)y, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData operator/(const Texture2DData &x, const Texture2DData &y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    check_if_types_match(x.type, y.type);
    if (x.type == COMPLEX || x.type == COMPLEX2) {
        tex_tex_complex_mul_tex(new_frame, x.frame, y.frame);
        return Texture2DData(x.type, new_frame, x.tex_params);
    }
    tex_tex_mul_tex(new_frame, x.frame, y.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData const Texture2DData::laplacian(struct Laplacian2DParams params) {
    frame_id new_frame = activate_frame(&this->tex_params);
    tex_laplacian2D(new_frame, this->frame, &params);
    return Texture2DData(this->type, new_frame, this->tex_params);
}


Texture2DData cos(const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_complex_cos(new_frame, x.frame);
    else
        tex_cos(new_frame, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData sin(const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_complex_sin(new_frame, x.frame);
    else
        tex_sin(new_frame, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData exp(const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_complex_exp(new_frame, x.frame);
    else
        tex_exp(new_frame, x.frame);
    return Texture2DData(x.type, new_frame, x.tex_params);
}

Texture2DData make_x(double x0, double xf, int type, int width, int height,
                     GLuint wrap_s, GLuint wrap_t,
                     GLuint min_filter, GLuint mag_filter) {
    struct TextureParams tex_params {.format=type_to_format(type),
        .width=(int)width, .height=(int)height,
        .wrap_s=(int)wrap_s, .wrap_t=(int)wrap_t,
        .min_filter=(int)min_filter, .mag_filter=(int)mag_filter};
    frame_id new_frame = activate_frame(&tex_params);
    struct Vec4 w00 = {{{.x=(float)x0, .y=(float)x0,
                .z=(float)x0, .w=(float)x0}}};
    struct Vec4 w10 = {{{.x=(float)xf, .y=(float)xf,
                .z=(float)xf, .w=(float)xf}}};
    struct Vec4 w01 = {{{.x=(float)x0, .y=(float)x0,
                .z=(float)x0, .w=(float)x0}}};
    struct Vec4 w11 = {{{.x=(float)xf, .y=(float)xf,
                .z=(float)xf, .w=(float)xf}}};
    tex_bilerp(new_frame, &w00, &w10, &w01, &w11);
    return Texture2DData(type, new_frame, tex_params);
}

Texture2DData make_y(double y0, double yf, int type, int width, int height,
                     GLuint wrap_s, GLuint wrap_t,
                     GLuint min_filter, GLuint mag_filter) {
    struct TextureParams tex_params {.format=type_to_format(type),
        .width=(int)width, .height=(int)height,
        .wrap_s=(int)wrap_s, .wrap_t=(int)wrap_t,
        .min_filter=(int)min_filter, .mag_filter=(int)mag_filter};
    frame_id new_frame = activate_frame(&tex_params);
    struct Vec4 w00 = {{{.x=(float)y0, .y=(float)y0,
                .z=(float)y0, .w=(float)y0}}};
    struct Vec4 w10 = {{{.x=(float)y0, .y=(float)y0,
                .z=(float)y0, .w=(float)y0}}};
    struct Vec4 w01 = {{{.x=(float)yf, .y=(float)yf,
                .z=(float)yf, .w=(float)yf}}};
    struct Vec4 w11 = {{{.x=(float)yf, .y=(float)yf,
                .z=(float)yf, .w=(float)yf}}};
    tex_bilerp(new_frame, &w00, &w10, &w01, &w11);
    return Texture2DData(type, new_frame, tex_params);
}

