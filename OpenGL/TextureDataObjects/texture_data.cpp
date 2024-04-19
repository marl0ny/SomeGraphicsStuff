#include "bin_ops.h"
#include "unary_ops.h"
#include "frames_stacks.h"
#include "fft.h"
#include "fft_omp.hpp"
#include "summation.h"
#include <GLES3/gl3.h>
#include <cmath>
#include <complex>
#include <iostream>
#include <map>
#include <string.h>
#include <string>
#include <vector>
// #include <vector>
#include <GLFW/glfw3.h>
#include "texture_data.hpp"
#include "serialize.h"


static int s_texture_data_ref_count = 0; 

static bool is_scalar_type(GLuint type) {
    return (type == FLOAT || type == HALF_FLOAT || type == SHORT ||
            type == USHORT || type == INT || type == UINT || type == BYTE ||
            type == UBYTE);
}

static int size_of_type(GLuint type) {
    switch(type) {
        case FLOAT: case HALF_FLOAT: case SHORT: case USHORT:
        case INT: case UINT:
        case BYTE: case UBYTE:
        return 1;
        case FLOAT2: case HALF_FLOAT2: case COMPLEX: case SHORT2: case USHORT2:
        case INT2: case UINT2: case BYTE2: case UBYTE2:
        return 2;
        case FLOAT3: case HALF_FLOAT3: case SHORT3: case USHORT3:
        case INT3: case UINT3: case BYTE3: case UBYTE3:
        return 3;

    }
    return 4;
}

static int single_channel_equiv_of_type(GLuint format) {
    switch(format) {
        case FLOAT: case FLOAT2: case FLOAT3: case FLOAT4:
        return FLOAT;
        case HALF_FLOAT: case HALF_FLOAT2:
        case HALF_FLOAT3: case HALF_FLOAT4:
        return HALF_FLOAT;
        case INT: case INT2: case INT3: case INT4:
        return INT;
        case UINT: case UINT2: case UINT3: case UINT4:
        return UINT;
        case USHORT: case USHORT2: case USHORT3: case USHORT4:
        return USHORT;
        case UBYTE: case UBYTE2: case UBYTE3: case UBYTE4:
        return UBYTE;
        case BYTE: case BYTE2: case BYTE3: case BYTE4:
        return BYTE;
    }
    return -1;
}

// TODO: complete this.
static int format_to_type(GLuint format) {
    switch(format) {
        case GL_R32F:
        return FLOAT;
        case GL_RG32F:
        return FLOAT2;
        case GL_RGB32F:
        return FLOAT3;
        case GL_RGBA32F:
        return FLOAT4;
        case GL_R16F:
        return HALF_FLOAT;
        case GL_RG16F:
        return HALF_FLOAT2;
        case GL_RGB16F:
        return HALF_FLOAT3;
        case GL_RGBA16F:
        return HALF_FLOAT4;
    }
    return -1;
}

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

static void modify_viewport_if_mismatch(int x, int y, int width, int height) {
    int data[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_VIEWPORT, data);
    if (data[0] != x && data[1] != y &&
        data[2] != width && data[3] != height) {
        glViewport(x, y, width, height);
        std::fprintf(stdout,
                     "Viewport does not match dimensions "
                     "of Texture2DData object. "
                     "Changing viewport values x, y, width, and height "
                     "from %d, %d, %d, %d to %d, %d, %d, %d.\n",
                     data[0], data[1], data[2], data[3],
                     x, y, width, height);
     }
}

void Texture2DData::increment_ref_count() {
    s_texture_data_ref_count++;
}

void Texture2DData::decrement_ref_count() {
    s_texture_data_ref_count--;
}

static void dimensions_copy(struct IVec4 &d1, const struct IVec4 &d2) {
    d1.x = d2.x;
    d1.y = d2.y;
    d1.z = d2.z;
    d1.w = d2.w;
}

static void set_dimensions(struct IVec4 &d, int x) {
    d.x = x;
    d.y = 0;
    d.z = 0;
    d.w = 0;
}

static void set_dimensions(struct IVec4 &d, int x, int y) {
    d.x = x;
    d.y = y;
    d.z = 0;
    d.w = 0;
}

static void set_dimensions(struct IVec4 &d, int x, int y, int z) {
    d.x = x;
    d.y = y;
    d.z = z;
    d.w = 0;
}

static void set_dimensions(struct IVec4 &d, int x, int y, int z, int w) {
    d.x = x;
    d.y = y;
    d.z = z;
    d.w = w;
}

static struct IVec4 make_new_dimensions(int x) {
    return {{{.x=x, .y=0, .z=0, .w=0}}};
}

static struct IVec4 make_new_dimensions(int x, int y) {
    return {{{.x=x, .y=y, .z=0, .w=0}}};
}

static struct IVec4 make_new_dimensions(int x, int y, int z) {
    return {{{.x=x, .y=y, .z=z, .w=0}}};
}

static struct IVec4 make_new_dimensions(int x, int y, int z, int w) {
    return {{{.x=x, .y=y, .z=z, .w=w}}};
}

Texture2DData::Texture2DData(int type, const std::string &fname) {
    this->frame = ::deserialize(fname.c_str(), &this->tex_params);
    this->type = type;
    set_dimensions(this->dimensions, 
        this->tex_params.width, this->tex_params.height);
    increment_ref_count();
}

Texture2DData::Texture2DData(const std::string &fname) {
    this->frame = ::deserialize(fname.c_str(), &this->tex_params);
    this->type = format_to_type(tex_params.format);
    set_dimensions(this->dimensions, 
        this->tex_params.width, this->tex_params.height);
    increment_ref_count();
}

Texture2DData::Texture2DData(int type, frame_id frame,
                             const struct IVec4 &dimensions,
                             const struct TextureParams &tex_params) {
    modify_viewport_if_mismatch(0, 0, tex_params.width, tex_params.height);
    this->type = type;
    this->frame = frame;
    dimensions_copy(this->dimensions, dimensions);
    this->tex_params = tex_params;
    increment_ref_count();
}

Texture2DData::Texture2DData(float *data, int width, int height,
                             bool generate_mipmap,
                             GLuint wrap_s, GLuint wrap_t,
                             GLuint min_filter, GLuint mag_filter) {
    modify_viewport_if_mismatch(0, 0, width, height);
    this->type = FLOAT;
    this->tex_params = {
        .format=type_to_format(type),
        .width=width, .height=height,
        .generate_mipmap = (int)generate_mipmap,
        .wrap_s = (int)wrap_s, .wrap_t = (int)wrap_t,
        .min_filter = (int)min_filter, .mag_filter = (int)mag_filter,
    };
    frame = activate_frame(&tex_params);
    set_dimensions(this->dimensions, width, height);
    tex_zero(frame);
    quad_substitute_array(frame, &this->tex_params, data);
    increment_ref_count();
}

Texture2DData::Texture2DData(std::complex<float> *data,
                             int width, int height,
                             bool generate_mipmap,
                             GLuint wrap_s, GLuint wrap_t,
                             GLuint min_filter, GLuint mag_filter) {
    modify_viewport_if_mismatch(0, 0, width, height);
    this->type = COMPLEX;
    this->tex_params = {
        .format=type_to_format(type),
        .width=width, .height=height,
        .generate_mipmap = (int)generate_mipmap,
        .wrap_s = (int)wrap_s, .wrap_t = (int)wrap_t,
        .min_filter = (int)min_filter, .mag_filter = (int)mag_filter,
    };
    frame = activate_frame(&tex_params);
    tex_zero(frame);
    set_dimensions(this->dimensions, width, height);
    quad_substitute_array(frame, &this->tex_params, data);
    increment_ref_count();
}

Texture2DData::Texture2DData(struct Vec2 *data, int width, int height,
                             bool generate_mipmap,
                             GLuint wrap_s, GLuint wrap_t,
                             GLuint min_filter, GLuint mag_filter) {
    modify_viewport_if_mismatch(0, 0, width, height);
    this->type = FLOAT2;
    this->tex_params = {
        .format=type_to_format(type),
        .width=width, .height=height,
        .generate_mipmap = (int)generate_mipmap,
        .wrap_s = (int)wrap_s, .wrap_t = (int)wrap_t,
        .min_filter = (int)min_filter, .mag_filter = (int)mag_filter,
    };
    frame = activate_frame(&tex_params);
    set_dimensions(this->dimensions, width, height);
    tex_zero(frame);
    quad_substitute_array(frame, &this->tex_params, data);
    increment_ref_count();
}

Texture2DData::Texture2DData(struct Vec3 *data, int width, int height,
                             bool generate_mipmap,
                             GLuint wrap_s, GLuint wrap_t,
                             GLuint min_filter, GLuint mag_filter) {
    modify_viewport_if_mismatch(0, 0, width, height);
    this->type = FLOAT3;
    this->tex_params = {
        .format=type_to_format(type),
        .width=width, .height=height,
        .generate_mipmap = (int)generate_mipmap,
        .wrap_s = (int)wrap_s, .wrap_t = (int)wrap_t,
        .min_filter = (int)min_filter, .mag_filter = (int)mag_filter,
    };
    set_dimensions(this->dimensions, width, height);
    frame = activate_frame(&tex_params);
    tex_zero(frame);
    quad_substitute_array(frame, &this->tex_params, data);
    increment_ref_count();
}

Texture2DData::Texture2DData(struct Vec4 *data, int width, int height,
                             bool generate_mipmap,
                             GLuint wrap_s, GLuint wrap_t,
                             GLuint min_filter, GLuint mag_filter) {
    modify_viewport_if_mismatch(0, 0, width, height);
    this->type = FLOAT4;
    this->tex_params = {
        .format=type_to_format(type),
        .width=width, .height=height,
        .generate_mipmap = (int)generate_mipmap,
        .wrap_s = (int)wrap_s, .wrap_t = (int)wrap_t,
        .min_filter = (int)min_filter, .mag_filter = (int)mag_filter,
    };
    set_dimensions(this->dimensions, width, height);
    frame = activate_frame(&tex_params);
    tex_zero(frame);
    quad_substitute_array(frame, &this->tex_params, data);
    increment_ref_count();
}

Texture2DData::Texture2DData(struct Uint8Vec4 *data, int width, int height,
                             bool generate_mipmap,
                             GLuint wrap_s, GLuint wrap_t,
                             GLuint min_filter, GLuint mag_filter) {
    modify_viewport_if_mismatch(0, 0, width, height);
    this->type = BYTE4;
    this->tex_params = {
        .format=type_to_format(type),
        .width=width, .height=height,
        .generate_mipmap = (int)generate_mipmap,
        .wrap_s = (int)wrap_s, .wrap_t = (int)wrap_t,
        .min_filter = (int)min_filter, .mag_filter = (int)mag_filter,
    };
    set_dimensions(this->dimensions, width, height);
    frame = activate_frame(&tex_params);
    tex_zero(frame);
    quad_substitute_array(frame, &this->tex_params, data);
    increment_ref_count();
}

Texture2DData::Texture2DData(int type, int width, int height,
                  bool generate_mipmap,
                  GLuint wrap_s, GLuint wrap_t,
                  GLuint min_filter, GLuint mag_filter) {
    modify_viewport_if_mismatch(0, 0, width, height);
    this->type = type;
    this->tex_params = {
        .format=type_to_format(type),
        .width=width, .height=height,
        .generate_mipmap = (int)generate_mipmap,
        .wrap_s = (int)wrap_s, .wrap_t = (int)wrap_t,
        .min_filter = (int)min_filter, .mag_filter = (int)mag_filter,
    };
    set_dimensions(this->dimensions, width, height);
    frame = activate_frame(&tex_params);
    increment_ref_count();
}

Texture2DData::Texture2DData(int type, int width, int height, int length,
                             bool generate_mipmap,
                             GLuint wrap_s, GLuint wrap_t,
                             GLuint min_filter, GLuint mag_filter) {
    modify_viewport_if_mismatch(0, 0, width, height);
    this->type = type;
    this->tex_params = {
        .format=type_to_format(type),
        .width=width*length, .height=height,
        .generate_mipmap = (int)generate_mipmap,
        .wrap_s = (int)wrap_s, .wrap_t = (int)wrap_t,
        .min_filter = (int)min_filter, .mag_filter = (int)mag_filter,
    };
    set_dimensions(this->dimensions, width, height, length);
    frame = activate_frame(&tex_params);
    increment_ref_count();
}

Texture2DData::Texture2DData(const Texture2DData &x) {
    modify_viewport_if_mismatch(0, 0, x.tex_params.width, x.tex_params.height);
    this->type = x.type;
    this->tex_params = {
        .format=x.tex_params.format,
        .width=x.tex_params.width, .height=x.tex_params.height,
        .generate_mipmap = x.tex_params.generate_mipmap,
        .wrap_s = x.tex_params.wrap_s, .wrap_t = x.tex_params.wrap_t,
        .min_filter = x.tex_params.min_filter,
        .mag_filter = x.tex_params.mag_filter,
    };
    dimensions_copy(this->dimensions, x.dimensions);
    frame = activate_frame(&tex_params);
    tex_copy(this->frame, x.frame);
    increment_ref_count();
}

Texture2DData& Texture2DData::operator=(const Texture2DData &x) {
    modify_viewport_if_mismatch(0, 0, x.tex_params.width, x.tex_params.height);
    // std::cout << "copy assignment called." << std::endl;
    tex_copy(this->frame, x.frame);
    dimensions_copy(this->dimensions, x.dimensions);
    return *this;
}

Texture2DData::Texture2DData(Texture2DData &&x) {
    modify_viewport_if_mismatch(0, 0, x.tex_params.width, x.tex_params.height);
    // Constructor so frame should be uninitialized.
    this->frame = x.frame;
    this->type = x.type;
    dimensions_copy(this->dimensions, x.dimensions);
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

Texture2DData &Texture2DData::operator=(Texture2DData &&x) {
    modify_viewport_if_mismatch(0, 0, x.tex_params.width, x.tex_params.height);
    deactivate_frame(&this->tex_params, this->frame);
    this->frame = x.frame;
    this->type = x.type;
    dimensions_copy(this->dimensions, x.dimensions);
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

void Texture2DData::paste_to_quad(frame_id quad) const {
    tex_copy(quad, this->frame);
}

void Texture2DData::paste_to_array(void *array) const {
    get_quad_array(this->frame, &this->tex_params, array);
}

void Texture2DData::paste_to_rgb_image_data(unsigned char *array) const {
    get_rgb_unsigned_byte_array(this->frame,
                                this->tex_params.width, this->tex_params.height,
                                array);
}

struct PixelData Texture2DData::sum_reduction() const {
    return sum(this->frame, &this->tex_params);
}

Texture2DData Texture2DData::transpose() const {
    struct TextureParams tex_params;
    copy_tex_params(&tex_params, &this->tex_params);
    tex_params.width = this->tex_params.height;
    tex_params.height = this->tex_params.width;
    frame_id new_frame = activate_frame(&tex_params);
    int tmp_viewport[4] = {0,};
    glGetIntegerv(GL_VIEWPORT, tmp_viewport);
    glViewport(0, 0, tex_params.width, tex_params.height);
    tex_transpose(new_frame, this->frame);
    glViewport(tmp_viewport[0], tmp_viewport[1],
               tmp_viewport[2], tmp_viewport[3]);
    return Texture2DData(this->type, new_frame,
                         make_new_dimensions(tex_params.width, tex_params.height),
                         tex_params);
}

Texture2DData Texture2DData::reduce_to_column() const {
    struct TextureParams tex_params;
    copy_tex_params(&tex_params, &this->tex_params);
    tex_params.width = 1;
    frame_id new_frame = activate_frame(&tex_params);
    reduce_2d_to_1d(new_frame, this->frame,
                    &tex_params, &this->tex_params);
    return Texture2DData(type, new_frame, 
        make_new_dimensions(tex_params.width, tex_params.height), tex_params);
}

Texture2DData Texture2DData::reduce_to_row() const {
    struct TextureParams tex_params;
    copy_tex_params(&tex_params, &this->tex_params);
    tex_params.height = 1;
    frame_id new_frame = activate_frame(&tex_params);
    reduce_2d_to_1d(new_frame, this->frame,
                    &tex_params, &this->tex_params);
    return Texture2DData(type, new_frame, 
             make_new_dimensions(tex_params.width, tex_params.height),
              tex_params);
}

struct PixelData Texture2DData::squared_norm() const {
    return norm_squared(this->frame, &this->tex_params);
}

Texture2DData Texture2DData::reduce_to_single_channel() const {
    struct TextureParams tex_params;
    copy_tex_params(&tex_params, &this->tex_params);
    int type = single_channel_equiv_of_type(this->type);
    int size = size_of_type(this->type);
    tex_params.format = type_to_format(type);
    frame_id new_frame = activate_frame(&tex_params);
    ::reduce_to_single_channel(new_frame, this->frame, size);
    return Texture2DData(type, new_frame, this->dimensions, tex_params);
}

void Texture2DData::set_as_sampler2D_uniform(const char *name) const {
    set_sampler2D_uniform(name, this->frame);
}

void Texture2DData::debug_print_state() const {
    std::cout << "type_id: " << type << "\n";
    std::cout << "frame_id: " << frame << "\n";
}

int Texture2DData::get_frame_id() const {
    return frame;
}

int Texture2DData::get_type_id() const {
    return type;
}

Texture2DData Texture2DData::cast_to(int type, Channel c0) const {
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
    return Texture2DData(type, new_frame, this->dimensions, tex_params);
}

Texture2DData Texture2DData::cast_to(int type, Channel c0, Channel c1) const {
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
    return Texture2DData(type, new_frame, this->dimensions, tex_params);
}

Texture2DData Texture2DData::cast_to(int type,
                                     Channel c0, Channel c1, Channel c2
                                    ) const {
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
    return Texture2DData(type, new_frame, this->dimensions, tex_params);
}

Texture2DData Texture2DData::cast_to(int type,
                                     Channel c0, Channel c1,
                                     Channel c2, Channel c3) const {
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
    return Texture2DData(type, new_frame, this->dimensions, tex_params);
}

void Texture2DData::serialize() const {
    ::serialize("file.dat", this->frame, &this->tex_params);
}

void Texture2DData::serialize(const std::string &path) const {
    ::serialize(path.c_str(), this->frame, &this->tex_params);
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
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator-(const Texture2DData &x, const Texture2DData &y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    check_if_types_match(x.type, y.type);
    tex_tex_sub_tex(new_frame, x.frame, y.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator*(const Texture2DData &x, const Texture2DData &y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    check_if_types_match(x.type, y.type);
    if ((x.type == COMPLEX && y.type == COMPLEX) ||
        (x.type == COMPLEX2 && y.type == COMPLEX2) ||
        (x.type == COMPLEX2 && y.type == COMPLEX) ||
        (x.type == COMPLEX && y.type == COMPLEX2)) {
        tex_tex_complex_mul_tex(new_frame, x.frame, y.frame);
        return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
    }
    tex_tex_mul_tex(new_frame, x.frame, y.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator+(const Texture2DData &x, struct Vec2 &v) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_tex_add_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator+(struct Vec2 &v, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_vec2_add_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator-(const Texture2DData &x, struct Vec2 &v) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_tex_sub_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator-(struct Vec2 &v, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_vec2_sub_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator*(const Texture2DData &x, struct Vec2 &v) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_tex_mul_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator*(struct Vec2 &v, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_vec2_mul_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator/(const Texture2DData &x, struct Vec2 &v) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_tex_div_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator/(struct Vec2 &v, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_vec2_div_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator+(const Texture2DData &x, std::complex<double> z) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_tex_add_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator+(std::complex<double> z, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_vec2_add_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator-(const Texture2DData &x, std::complex<double> z) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_tex_sub_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator-(std::complex<double> z, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_vec2_sub_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator*(const Texture2DData &x, std::complex<double> z) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_tex_complex_mul_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator*(std::complex<double> z, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_vec2_complex_mul_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator/(const Texture2DData &x, std::complex<double> z) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_tex_complex_div_vec2(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator/(std::complex<double> z, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    struct Vec2 v = {{{.x=(float)std::real(z), .y=(float)std::imag(z)}}};
    tex_vec2_complex_div_tex(new_frame, &v, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator+(const Texture2DData &x, double y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_tex_complex_add_float(new_frame, x.frame, (float)y);
    else
        tex_tex_add_float(new_frame, x.frame, (float)y);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator+(double y, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_float_complex_add_tex(new_frame, (float)y, x.frame);
    else
        tex_float_add_tex(new_frame, (float)y, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator-(const Texture2DData &x, double y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_tex_complex_sub_float(new_frame, x.frame, (float)y);
    else
        tex_tex_sub_float(new_frame, x.frame, (float)y);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator-(double y, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_float_complex_sub_tex(new_frame, (float)y, x.frame);
    else
        tex_float_sub_tex(new_frame, (float)y, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator*(const Texture2DData &x, double y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_tex_complex_mul_float(new_frame, x.frame, (float)y);
    else
        tex_tex_mul_float(new_frame, x.frame, (float)y);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator*(double y, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_float_complex_mul_tex(new_frame, (float)y, x.frame);
    else
        tex_float_mul_tex(new_frame, (float)y, x.frame);;
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator/(const Texture2DData &x, double y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_tex_complex_div_float(new_frame, x.frame, (float)y);
    else
        tex_tex_div_float(new_frame, x.frame, (float)y);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator/(double y, const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_float_complex_div_tex(new_frame, (float)y, x.frame);
    else
        tex_float_div_tex(new_frame, (float)y, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData operator/(const Texture2DData &x, const Texture2DData &y) {
    frame_id new_frame = activate_frame(&x.tex_params);
    check_if_types_match(x.type, y.type);
    if (x.type == COMPLEX || x.type == COMPLEX2) {
        tex_tex_complex_div_tex(new_frame, x.frame, y.frame);
        return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
    }
    tex_tex_mul_tex(new_frame, x.frame, y.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

/* Texture2DData Texture2DData::laplacian(struct Laplacian2DParams params) const {
    frame_id new_frame = activate_frame(&this->tex_params);
    tex_laplacian2D(new_frame, this->frame, &params);
    return Texture2DData(this->type, new_frame, this->dimensions, this->tex_params);
}*/

Texture2DData conj(const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2) {
        tex_conj(new_frame, x.frame);
    } else {
        tex_copy(new_frame, x.frame);
    }
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}


Texture2DData cos(const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_complex_cos(new_frame, x.frame);
    else
        tex_cos(new_frame, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData sin(const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_complex_sin(new_frame, x.frame);
    else
        tex_sin(new_frame, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData exp(const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_complex_exp(new_frame, x.frame);
    else
        tex_exp(new_frame, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData sqrt(const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    if (x.type == COMPLEX || x.type == COMPLEX2)
        tex_complex_sqrt(new_frame, x.frame);
    else
        tex_sqrt(new_frame, x.frame);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData pow(const Texture2DData &x, int n) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_pow(new_frame, x.frame, (double)n);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData pow(const Texture2DData &x, double n) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_pow(new_frame, x.frame, n);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData min(double a, const Texture2DData &v) {
    // if (is_scalar_type(v.type)) {
        frame_id new_frame = activate_frame(&v.tex_params);
        tex_float_min_tex(new_frame, a, v.frame);
        return Texture2DData(v.type, new_frame, v.dimensions, v.tex_params);
    /*} else {
        int size = size_of_Texture2DData maxtype(v.type);
        // TODO!
    }*/
}

Texture2DData min(const Texture2DData &v, double a) {
    return min(a, v);
}

Texture2DData max(double a, const Texture2DData &v) {
    // if (is_scalar_type(v.type)) {
        frame_id new_frame = activate_frame(&v.tex_params);
        tex_float_max_tex(new_frame, a, v.frame);
        return Texture2DData(v.type, new_frame, v.dimensions, v.tex_params);
    /* } else {
        int size = size_of_type(v.type);
        // TODO!
    }*/
}

Texture2DData max(const Texture2DData &v, double a) {
    return max(a, v);
}

Texture2DData funcs2D::zeroes(int type, int width, int height,
                              bool generate_mipmap,
                              GLuint wrap_s, GLuint wrap_t,
                              GLuint min_filter, GLuint mag_filter) {
    TextureParams tex_params {
        .format=type_to_format(type),
        .width=width, .height=height,
        .generate_mipmap = (int)generate_mipmap,
        .wrap_s = (int)wrap_s, .wrap_t = (int)wrap_t,
        .min_filter = (int)min_filter, .mag_filter = (int)mag_filter,
    };
    struct IVec4 dimensions = {.x=width, .y=height, .z=0, .w=0};
    frame_id new_frame = activate_frame(&tex_params);
    tex_zero(new_frame);
    return Texture2DData(type, new_frame, dimensions, tex_params);
}

Texture2DData funcs2D::make_y(double y0, double yf, 
                              int type, int width, int height,
                              GLuint wrap_s, GLuint wrap_t,
                              GLuint min_filter, GLuint mag_filter) {
    struct TextureParams tex_params {.format=type_to_format(type),
        .width=(int)width, .height=(int)height,
        .wrap_s=(int)wrap_s, .wrap_t=(int)wrap_t,
        .min_filter=(int)min_filter, .mag_filter=(int)mag_filter};
    struct IVec4 dimensions = {.x=width, .y=height, .z=0, .w=0};
    frame_id new_frame = activate_frame(&tex_params);
    struct Vec4 w00 = {{{.x=(float)y0, .y=(float)y0,
                .z=(float)y0, .w=(float)y0}}};
    struct Vec4 w10 = {{{.x=(float)yf, .y=(float)yf,
                .z=(float)yf, .w=(float)yf}}};
    struct Vec4 w01 = {{{.x=(float)y0, .y=(float)y0,
                .z=(float)y0, .w=(float)y0}}};
    struct Vec4 w11 = {{{.x=(float)yf, .y=(float)yf,
                .z=(float)yf, .w=(float)yf}}};
    tex_bilerp(new_frame, &w00, &w10, &w01, &w11);
    return Texture2DData(type, new_frame, dimensions, tex_params);
}

Texture2DData funcs2D::make_x(double x0, double xf, int type, 
                              int width, int height,
                              GLuint wrap_s, GLuint wrap_t,
                              GLuint min_filter, GLuint mag_filter) {
    struct TextureParams tex_params {.format=type_to_format(type),
        .width=(int)width, .height=(int)height,
        .wrap_s=(int)wrap_s, .wrap_t=(int)wrap_t,
        .min_filter=(int)min_filter, .mag_filter=(int)mag_filter};
    struct IVec4 dimensions = {.x=width, .y=height, .z=0, .w=0};
    frame_id new_frame = activate_frame(&tex_params);
    struct Vec4 w00 = {{{.x=(float)x0, .y=(float)x0,
                .z=(float)x0, .w=(float)x0}}};
    struct Vec4 w10 = {{{.x=(float)x0, .y=(float)x0,
                .z=(float)x0, .w=(float)x0}}};
    struct Vec4 w01 = {{{.x=(float)xf, .y=(float)xf,
                .z=(float)xf, .w=(float)xf}}};
    struct Vec4 w11 = {{{.x=(float)xf, .y=(float)xf,
                .z=(float)xf, .w=(float)xf}}};
    // TODO: Fix the naming conventions for the variables w00...w11
    // used inside this function as they do not match the convention
    // used in tex_bilerp.
    tex_bilerp(new_frame, &w00, &w10, &w01, &w11);
    return Texture2DData(type, new_frame, dimensions, tex_params);
}

Texture2DData funcs2D::ddx(const Texture2DData &f, struct Grad2DParams params) {
    frame_id new_frame = activate_frame(&f.tex_params);
    tex_ddx(new_frame, f.frame, &params);
    return Texture2DData(f.type, new_frame, f.dimensions, f.tex_params);
}

Texture2DData funcs2D::ddy(const Texture2DData &f, struct Grad2DParams params) {
    frame_id new_frame = activate_frame(&f.tex_params);
    tex_ddy(new_frame, f.frame, &params);
    return Texture2DData(f.type, new_frame, f.dimensions, f.tex_params);
}

Texture2DData funcs2D::laplacian(
    const Texture2DData &f, struct Laplacian2DParams params) {
    frame_id new_frame = activate_frame(&f.tex_params);
    tex_laplacian2D(new_frame, f.frame, &params);
    return Texture2DData(f.type, new_frame, f.dimensions, f.tex_params);
}

Texture2DData funcs2D::fft(const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_fft(new_frame, x.frame, &x.tex_params);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData funcs2D::ifft(const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_ifft(new_frame, x.frame, &x.tex_params);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData funcs2D::fft_omp(const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData funcs2D::ifft_omp(const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData funcs2D::fftshift(const Texture2DData &x) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_fftshift(new_frame, x.frame, &x.tex_params);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData funcs2D::fftshift(const Texture2DData &x, int type) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_fftshift(new_frame, x.frame, &x.tex_params);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData funcs2D::roll(const Texture2DData &x, struct Vec2 v) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_roll(new_frame, x.frame, &v);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

Texture2DData funcs2D::roll(const Texture2DData &x, double a, double b) {
    struct Vec2 v;
    v.x = (float)a;
    v.y = (float)b;
    return funcs2D::roll(x, v);
}

namespace funcs3D {

    Texture2DData zeroes(int type, int width, int height, int length,
                         bool generate_mipmap,
                         GLuint wrap_s, GLuint wrap_t,
                         GLuint min_filter, 
                         GLuint mag_filter) {
        IVec3 dimensions_3d {.width=width, .height=height, .length=length};
        IVec2 tex_dimensions_2d = get_2d_from_3d_dimensions(&dimensions_3d);
        TextureParams tex_params {
            .format=type_to_format(type),
            .width=tex_dimensions_2d.width,
            .height=tex_dimensions_2d.height,
            .generate_mipmap = (int)generate_mipmap,
            .wrap_s = (int)wrap_s, .wrap_t = (int)wrap_t,
            .min_filter = (int)min_filter, .mag_filter = (int)mag_filter,
        };
        frame_id new_frame = activate_frame(&tex_params);
        tex_zero(new_frame);
        struct IVec4 dimensions = {.x=width, .y=height, .z=length, .w=0};
        return Texture2DData(type, new_frame, dimensions, tex_params);
    }

    Texture2DData make_x(double x0, double xf, 
                         int type, 
                         int width, int height, int length,
                         GLuint wrap_s, GLuint wrap_t,
                         GLuint min_filter, GLuint mag_filter) {
        IVec3 dimensions_3d {.width=width, .height=height, .length=length};
        IVec2 tex_dimensions_2d = get_2d_from_3d_dimensions(&dimensions_3d);
        struct TextureParams tex_params {.format=type_to_format(type),
            .width=(int)tex_dimensions_2d.width,
            .height=(int)tex_dimensions_2d.height,
            .wrap_s=(int)wrap_s, .wrap_t=(int)wrap_t,
            .min_filter=(int)min_filter, .mag_filter=(int)mag_filter};
        frame_id new_frame = activate_frame(&tex_params);
        struct IVec3 texture_dimensions_3d
             = {{{.x=width, .y=height, .z=length}}};
        // z = 0
        struct Vec4 w000 = {{{.x=(float)x0, .y=(float)x0,
                              .z=(float)x0, .w=(float)x0}}};
        struct Vec4 w100 = {{{.x=(float)xf, .y=(float)xf,
                              .z=(float)xf, .w=(float)xf}}};
        struct Vec4 w010 = {{{.x=(float)x0, .y=(float)x0,
                              .z=(float)x0, .w=(float)x0}}};
        struct Vec4 w110 = {{{.x=(float)xf, .y=(float)xf,
                              .z=(float)xf, .w=(float)xf}}};
        // z = z_max
        struct Vec4 w001 = {{{.x=(float)x0, .y=(float)x0,
                              .z=(float)x0, .w=(float)x0}}};
        struct Vec4 w101 = {{{.x=(float)xf, .y=(float)xf,
                              .z=(float)xf, .w=(float)xf}}};
        struct Vec4 w011 = {{{.x=(float)x0, .y=(float)x0,
                              .z=(float)x0, .w=(float)x0}}};
        struct Vec4 w111 = {{{.x=(float)xf, .y=(float)xf,
                              .z=(float)xf, .w=(float)xf}}};
        tex_trilerp(new_frame, &tex_dimensions_2d, &texture_dimensions_3d, 
                    &w000, &w010, &w100, &w110,
                    &w001, &w011, &w101, &w111);
        struct IVec4 dimensions = {.x=width, .y=height, .z=length, .w=0};
        return Texture2DData(type, new_frame, dimensions, tex_params);
    }

    Texture2DData make_y(double y0, double yf, 
                         int type, 
                         int width, int height, int length,
                         GLuint wrap_s, GLuint wrap_t,
                         GLuint min_filter, GLuint mag_filter) {
        IVec3 dimensions_3d {.width=width, .height=height, .length=length};
        IVec2 tex_dimensions_2d = get_2d_from_3d_dimensions(&dimensions_3d);
        struct TextureParams tex_params {.format=type_to_format(type),
            .width=(int)tex_dimensions_2d.width,
            .height=(int)tex_dimensions_2d.height,
            .wrap_s=(int)wrap_s, .wrap_t=(int)wrap_t,
            .min_filter=(int)min_filter, .mag_filter=(int)mag_filter};
        frame_id new_frame = activate_frame(&tex_params);
        struct IVec3 texture_dimensions_3d
             = {{{.x=width, .y=height, .z=length}}};
        // z = 0
        struct Vec4 w000 = {{{.x=(float)y0, .y=(float)y0,
                              .z=(float)y0, .w=(float)y0}}};
        struct Vec4 w100 = {{{.x=(float)y0, .y=(float)y0,
                              .z=(float)y0, .w=(float)y0}}};
        struct Vec4 w010 = {{{.x=(float)yf, .y=(float)yf,
                              .z=(float)yf, .w=(float)yf}}};
        struct Vec4 w110 = {{{.x=(float)yf, .y=(float)yf,
                              .z=(float)yf, .w=(float)yf}}};
        // z = z_max
        struct Vec4 w001 = {{{.x=(float)y0, .y=(float)y0,
                              .z=(float)y0, .w=(float)y0}}};
        struct Vec4 w101 = {{{.x=(float)y0, .y=(float)y0,
                              .z=(float)y0, .w=(float)y0}}};
        struct Vec4 w011 = {{{.x=(float)yf, .y=(float)yf,
                              .z=(float)yf, .w=(float)yf}}};
        struct Vec4 w111 = {{{.x=(float)yf, .y=(float)yf,
                              .z=(float)yf, .w=(float)yf}}};
        tex_trilerp(new_frame, &tex_dimensions_2d, &texture_dimensions_3d,
                    &w000, &w010, &w100, &w110,
                    &w001, &w011, &w101, &w111);
        struct IVec4 dimensions = {.x=width, .y=height, .z=length, .w=0};
        return Texture2DData(type, new_frame, dimensions, tex_params);
    }

    Texture2DData make_z(double z0, double zf, 
                         int type, 
                         int width, int height, int length,
                         GLuint wrap_s, GLuint wrap_t,
                         GLuint min_filter, GLuint mag_filter) {
        IVec3 dimensions_3d {.width=width, .height=height, .length=length};
        IVec2 tex_dimensions_2d = get_2d_from_3d_dimensions(&dimensions_3d);
        struct TextureParams tex_params {.format=type_to_format(type),
            .width=(int)tex_dimensions_2d.width,
            .height=(int)tex_dimensions_2d.height,
            .wrap_s=(int)wrap_s, .wrap_t=(int)wrap_t,
            .min_filter=(int)min_filter, .mag_filter=(int)mag_filter};
        frame_id new_frame = activate_frame(&tex_params);
        struct IVec3 texture_dimensions_3d
             = {{{.x=width, .y=height, .z=length}}};
        // z = 0
        struct Vec4 w000 = {{{.x=(float)z0, .y=(float)z0,
                              .z=(float)z0, .w=(float)z0}}};
        struct Vec4 w100 = {{{.x=(float)z0, .y=(float)z0,
                              .z=(float)z0, .w=(float)z0}}};
        struct Vec4 w010 = {{{.x=(float)z0, .y=(float)z0,
                              .z=(float)z0, .w=(float)z0}}};
        struct Vec4 w110 = {{{.x=(float)z0, .y=(float)z0,
                              .z=(float)z0, .w=(float)z0}}};
        // z = z_max
        struct Vec4 w001 = {{{.x=(float)zf, .y=(float)zf,
                              .z=(float)zf, .w=(float)zf}}};
        struct Vec4 w101 = {{{.x=(float)zf, .y=(float)zf,
                              .z=(float)zf, .w=(float)zf}}};
        struct Vec4 w011 = {{{.x=(float)zf, .y=(float)zf,
                              .z=(float)zf, .w=(float)zf}}};
        struct Vec4 w111 = {{{.x=(float)zf, .y=(float)zf,
                              .z=(float)zf, .w=(float)zf}}};
        tex_trilerp(new_frame, &tex_dimensions_2d, &texture_dimensions_3d,
                    &w000, &w010, &w100, &w110,
                    &w001, &w011, &w101, &w111);
        struct IVec4 dimensions = {.x=width, .y=height, .z=length, .w=0};
        return Texture2DData(type, new_frame, dimensions, tex_params);
    }

    Texture2DData fft(const Texture2DData &x) {
        frame_id new_frame = activate_frame(&x.tex_params);
        tex_fft3d(new_frame, x.frame, 
                  (struct IVec3 *)&x.dimensions, &x.tex_params);
        return Texture2DData(x.type, new_frame, 
                             x.dimensions, x.tex_params);
    }

    Texture2DData ifft(const Texture2DData &x) {
        frame_id new_frame = activate_frame(&x.tex_params);
        tex_ifft3d(new_frame, x.frame, 
                   (struct IVec3 *)&x.dimensions, &x.tex_params);
        return Texture2DData(x.type, new_frame, 
                             x.dimensions, x.tex_params);
    }

    Texture2DData fftshift(const Texture2DData &x) {
        frame_id new_frame = activate_frame(&x.tex_params);
        tex_fftshift3d(new_frame, x.frame, (struct IVec3 *)&x.dimensions, &x.tex_params);
        return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
    }

}

Texture2DData cast_to(int type,
                      const Texture2DData &x,
                      Channel c0, Channel c1, Channel c2, Channel c3,
                      const Texture2DData &y,
                      Channel d0, Channel d1, Channel d2, Channel d3) {
    struct TextureParams p {
        .format=type_to_format(type),
        .width = x.tex_params.width, .height = x.tex_params.height,
        .generate_mipmap = x.tex_params.generate_mipmap,
        .wrap_s = x.tex_params.wrap_s,
        .wrap_t = x.tex_params.wrap_t,
        .min_filter = x.tex_params.min_filter,
        .mag_filter = x.tex_params.mag_filter,
    };
    frame_id new_frame = activate_frame(&p);
    tex_swizzle2(new_frame,
                 x.frame, (int)c0, (int)c1, (int)c2, (int)c3,
                 y.frame, (int)d0, (int)d1, (int)d2, (int)d3);
    return Texture2DData(type, new_frame, x.dimensions, p);
}


Texture2DData substitute(const Texture2DData &x,
                         double old_val, double new_val) {
    frame_id new_frame = activate_frame(&x.tex_params);
    tex_substitute_float(new_frame, x.frame,
                         (float)old_val, (float)new_val);
    return Texture2DData(x.type, new_frame, x.dimensions, x.tex_params);
}

