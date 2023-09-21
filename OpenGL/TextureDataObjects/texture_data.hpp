#include "bin_ops.h"
#include "unary_ops.h"
#include "frames_stacks.h"
#include <cmath>
#include <complex>

#ifndef _TEXTURE_DATA_
#define _TEXTURE_DATA_

enum {
    FLOAT=0, FLOAT2, FLOAT3, FLOAT4,
    HALF_FLOAT, HALF_FLOAT2, HALF_FLOAT3, HALF_FLOAT4,
    COMPLEX, COMPLEX2,
    SHORT, SHORT2, SHORT3, SHORT4,
    USHORT, USHORT2, USHORT3, USHORT4,
    INT, INT2, INT3, INT4,
    UINT, UINT2, UINT3, UINT4,
    BYTE, BYTE2, BYTE3, BYTE4,
    UBYTE, UBYTE2, UBYTE3, UBYTE4,
};

enum Channel {
    NONE=-1,
    X=0, Y=1, Z=2, W=3,
    R=0, G=1, B=2, A=3,
};

class Texture2DData {
    int type = -1;
    frame_id frame = 0;
    struct TextureParams tex_params = {0,};
    // Addition
    friend Texture2DData operator+(const Texture2DData &x,
                                   const Texture2DData &y);
    friend Texture2DData operator+(const Texture2DData &x, double y);
    friend Texture2DData operator+(double x, const Texture2DData &y);
    friend Texture2DData operator+(const Texture2DData &x, struct Vec2 &y);
    friend Texture2DData operator+(struct Vec2 &x, const Texture2DData &y);
    friend Texture2DData operator+(const Texture2DData &x,
                                   std::complex<double> y);
    friend Texture2DData operator+(std::complex<double >x,
                                   const Texture2DData &y);
    friend Texture2DData operator+(const Texture2DData &x, struct Vec3 &y);
    friend Texture2DData operator+(struct Vec3 &x, const Texture2DData &y);
    friend Texture2DData operator+(const Texture2DData &x, struct Vec4 &y);
    friend Texture2DData operator+(struct Vec4 &x, const Texture2DData &y);
    // Subtraction
    friend Texture2DData operator-(const Texture2DData &x,
                                   const Texture2DData &y);
    friend Texture2DData operator-(const Texture2DData &x, double y);
    friend Texture2DData operator-(double x, const Texture2DData &y);
    friend Texture2DData operator-(const Texture2DData &x, struct Vec2 &y);
    friend Texture2DData operator-(struct Vec2 &x, const Texture2DData &y);
    friend Texture2DData operator-(const Texture2DData &x,
                                   std::complex<double> y);
    friend Texture2DData operator-(std::complex<double> x,
                                   const Texture2DData &y);
    friend Texture2DData operator-(const Texture2DData &x, struct Vec3 &y);
    friend Texture2DData operator-(struct Vec3 &x, const Texture2DData &y);
    friend Texture2DData operator-(const Texture2DData &x, struct Vec4 &y);
    friend Texture2DData operator-(struct Vec4 &x, const Texture2DData &y);
    // Multiplication
    friend Texture2DData operator*(const Texture2DData &x,
                                   const Texture2DData &y);
    friend Texture2DData operator*(const Texture2DData &x, double y);
    friend Texture2DData operator*(double x, const Texture2DData &y);
    friend Texture2DData operator*(const Texture2DData &x, struct Vec2 &y);
    friend Texture2DData operator*(struct Vec2 &x, const Texture2DData &y);
    friend Texture2DData operator*(const Texture2DData &x,
                                   std::complex<double> y);
    friend Texture2DData operator*(std::complex<double> x,
                                   const Texture2DData &y);
    friend Texture2DData operator*(const Texture2DData &x, struct Vec3 &y);
    friend Texture2DData operator*(struct Vec3 &x, const Texture2DData &y);
    friend Texture2DData operator*(const Texture2DData &x, struct Vec4 &y);
    friend Texture2DData operator*(struct Vec4 &x, const Texture2DData &y);
    // Division
    friend Texture2DData operator/(const Texture2DData &x,
                                   const Texture2DData &y);
    friend Texture2DData operator/(const Texture2DData &x, double y);
    friend Texture2DData operator/(double x, const Texture2DData &y);
    friend Texture2DData operator/(const Texture2DData &x, struct Vec2 &y);
    friend Texture2DData operator/(struct Vec2 &x, const Texture2DData &y);
    friend Texture2DData operator/(const Texture2DData &x,
                                   std::complex<double> y);
    friend Texture2DData operator/(std::complex<double> x,
                                   const Texture2DData &y);
    friend Texture2DData operator/(const Texture2DData &x, struct Vec3 &y);
    friend Texture2DData operator/(struct Vec3 &x, const Texture2DData &y);
    friend Texture2DData operator/(const Texture2DData &x, struct Vec4 &y);
    friend Texture2DData operator/(struct Vec4 &x, const Texture2DData &y);
    friend void swap(Texture2DData &, Texture2DData &);
    friend Texture2DData conj(const Texture2DData &x);
    friend Texture2DData cos(const Texture2DData &x);
    friend Texture2DData sin(const Texture2DData &x);
    friend Texture2DData exp(const Texture2DData &x);
    friend Texture2DData sqrt(const Texture2DData &x);
    friend Texture2DData pow(const Texture2DData &x, int n);
    friend Texture2DData pow(const Texture2DData &x, double n);
    friend Texture2DData min(double a, const Texture2DData &v);
    friend Texture2DData min(const Texture2DData &v, double a);
    friend Texture2DData max(double a, const Texture2DData &v);
    friend Texture2DData max(const Texture2DData &v, double a);
    friend Texture2DData make_x(double x0, double xf,
                                int type, int width, int height,
                                GLuint wrap_s, GLuint wrap_t,
                                GLuint min_filter, GLuint mag_filter);
    friend Texture2DData make_y(double y0, double yf,
                                int type, int width, int height,
                                GLuint wrap_s, GLuint wrap_t,
                                GLuint min_filter, GLuint mag_filter);
    friend Texture2DData ddx(const Texture2DData &f,
                             struct Grad2DParams params);
    friend Texture2DData ddy(const Texture2DData &f,
                             struct Grad2DParams params);
    friend Texture2DData fft(const Texture2DData &x);
    friend Texture2DData ifft(const Texture2DData &x);
    friend Texture2DData fft_omp(const Texture2DData &x);
    friend Texture2DData ifft_omp(const Texture2DData &x);
    friend Texture2DData fftshift(const Texture2DData &x);
    friend Texture2DData fftshift(const Texture2DData &x, int type);
    friend Texture2DData cast_to(int type,
                                 const Texture2DData &x,
                                 Channel c0, Channel c1,
                                 Channel c2, Channel c3,
                                 const Texture2DData &y,
                                 Channel d0, Channel d1,
                                 Channel d2, Channel d3);
    friend Texture2DData roll(const Texture2DData &x, struct Vec2 v);
    friend Texture2DData roll(const Texture2DData &x, double a, double b);
    friend Texture2DData substitute(const Texture2DData &x,
                                    double old_val, double new_val);
    friend Texture2DData zeroes(int type, int width, int height,
                                bool generate_mipmap,
                                GLuint wrap_s, GLuint wrap_t,
                                GLuint min_filter, GLuint mag_filter);
    friend class DrawTexture2DData;
    void increment_ref_count();
    void decrement_ref_count();
    Texture2DData(int type, frame_id frame,
                  const struct TextureParams &tex_params);
public:
    Texture2DData(float *data, int width, int height, bool generate_mipmap=true,
                  GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                  GLuint min_filter=GL_LINEAR, GLuint mag_filter=GL_LINEAR);
    Texture2DData(std::complex<float> *data, int width, int height, 
                  bool generate_mipmap=true,
                  GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                  GLuint min_filter=GL_LINEAR, GLuint mag_filter=GL_LINEAR);
    Texture2DData(struct Vec2 *data, int width, int height,
                  bool generate_mipmap=true,
                  GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                  GLuint min_filter=GL_LINEAR, GLuint mag_filter=GL_LINEAR);
    Texture2DData(struct Vec3 *data, int width, int height,
                  bool generate_mipmap=true,
                  GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                  GLuint min_filter=GL_LINEAR, GLuint mag_filter=GL_LINEAR);
    Texture2DData(struct Vec4 *data, int width, int height,
                  bool generate_mipmap=true,
                  GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                  GLuint min_filter=GL_LINEAR, GLuint mag_filter=GL_LINEAR);
    Texture2DData(struct Uint8Vec4 *data, int width, int height,
                  bool generate_mipmap=true,
                  GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                  GLuint min_filter=GL_LINEAR, GLuint mag_filter=GL_LINEAR);
    Texture2DData(int type, int width, int height,
                  bool generate_mipmap=true,
                  GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                  GLuint min_filter=GL_LINEAR, GLuint mag_filter=GL_LINEAR);
    Texture2DData(int type, const std::string &path);
    Texture2DData(const std::string &path);
    Texture2DData(const Texture2DData &x);
    Texture2DData &operator=(const Texture2DData &x);
    Texture2DData(Texture2DData &&x);
    Texture2DData &operator=(Texture2DData &&x);
    void paste_to_quad(frame_id quad) const;
    void debug_print_state() const;
    int get_frame_id() const;
    int get_type_id() const;
    void set_as_sampler2D_uniform(const char *name) const;
    Texture2DData laplacian(struct Laplacian2DParams params) const;
    Texture2DData cast_to(int type, Channel c0) const;
    Texture2DData cast_to(int type, Channel c0, Channel c1) const;
    Texture2DData cast_to(int type, Channel c0, Channel c1, Channel c2) const;
    Texture2DData cast_to(int type,
                          Channel c0, Channel c1,
                          Channel c2, Channel c3) const;
    void paste_to_array(void *array) const;
    void paste_to_rgb_image_data(unsigned char *array) const;
    void paste_to_rgba_image_data(unsigned char *array) const;
    struct PixelData sum_reduction() const;
    Texture2DData transpose() const;
    Texture2DData reduce_to_column() const;
    Texture2DData reduce_to_row() const;
    Texture2DData reduce_to_single_channel() const;
    struct PixelData squared_norm() const;
    void serialize() const;
    void serialize(const std::string &path) const;
    ~Texture2DData() {
        decrement_ref_count();
        if (frame >= 0)
            deactivate_frame(&tex_params, frame);
    }
};


Texture2DData operator+(const Texture2DData &x,
                        const Texture2DData &y);
Texture2DData operator+(const Texture2DData &x, double y);
Texture2DData operator+(double x, const Texture2DData &y);
Texture2DData operator+(const Texture2DData &x, struct Vec2 &y);
Texture2DData operator+(struct Vec2 &x, const Texture2DData &y);
Texture2DData operator+(const Texture2DData &x,
                        std::complex<double> y);
Texture2DData operator+(std::complex<double >x,
                        const Texture2DData &y);
Texture2DData operator+(const Texture2DData &x, struct Vec3 &y);
Texture2DData operator+(struct Vec3 &x, const Texture2DData &y);
Texture2DData operator+(const Texture2DData &x, struct Vec4 &y);
Texture2DData operator+(struct Vec4 &x, const Texture2DData &y);
// Subtraction
Texture2DData operator-(const Texture2DData &x,
                        const Texture2DData &y);
Texture2DData operator-(const Texture2DData &x, double y);
Texture2DData operator-(double x, const Texture2DData &y);
Texture2DData operator-(const Texture2DData &x, struct Vec2 &y);
Texture2DData operator-(struct Vec2 &x, const Texture2DData &y);
Texture2DData operator-(const Texture2DData &x, 
                        std::complex<double> y);
Texture2DData operator-(std::complex<double> x,
                        const Texture2DData &y);
Texture2DData operator-(const Texture2DData &x, struct Vec3 &y);
Texture2DData operator-(struct Vec3 &x, const Texture2DData &y);
Texture2DData operator-(const Texture2DData &x, struct Vec4 &y);
Texture2DData operator-(struct Vec4 &x, const Texture2DData &y);
// Multiplication
Texture2DData operator*(const Texture2DData &x,
                        const Texture2DData &y);
Texture2DData operator*(const Texture2DData &x, double y);
Texture2DData operator*(double x, const Texture2DData &y);
Texture2DData operator*(const Texture2DData &x, struct Vec2 &y);
Texture2DData operator*(struct Vec2 &x, const Texture2DData &y);
Texture2DData operator*(const Texture2DData &x,
                        std::complex<double> y);
Texture2DData operator*(std::complex<double> x,
                        const Texture2DData &y);
Texture2DData operator*(const Texture2DData &x, struct Vec3 &y);
Texture2DData operator*(struct Vec3 &x, const Texture2DData &y);
Texture2DData operator*(const Texture2DData &x, struct Vec4 &y);
Texture2DData operator*(struct Vec4 &x, const Texture2DData &y);
// Division
Texture2DData operator/(const Texture2DData &x,
                        const Texture2DData &y);
Texture2DData operator/(const Texture2DData &x, double y);
Texture2DData operator/(double x, const Texture2DData &y);
Texture2DData operator/(const Texture2DData &x, struct Vec2 &y);
Texture2DData operator/(struct Vec2 &x, const Texture2DData &y);
Texture2DData operator/(const Texture2DData &x,
                        std::complex<double> y);
Texture2DData operator/(std::complex<double> x,
                        const Texture2DData &y);
Texture2DData operator/(const Texture2DData &x, struct Vec3 &y);
Texture2DData operator/(struct Vec3 &x, const Texture2DData &y);
Texture2DData operator/(const Texture2DData &x, struct Vec4 &y);
Texture2DData operator/(struct Vec4 &x, const Texture2DData &y);

void swap(Texture2DData &, Texture2DData &);

Texture2DData conj(const Texture2DData &x);

Texture2DData cos(const Texture2DData &x);

Texture2DData sin(const Texture2DData &x);

Texture2DData exp(const Texture2DData &x);

Texture2DData sqrt(const Texture2DData &x);

Texture2DData pow(const Texture2DData &x, int n);

Texture2DData pow(const Texture2DData &x, double n);

Texture2DData min(double a, const Texture2DData &v);

Texture2DData min(const Texture2DData &v, double a);

Texture2DData max(double a, const Texture2DData &v);

Texture2DData max(const Texture2DData &v, double a);

Texture2DData make_x(double x0, double xf, int type, int width, int height,
                     GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                     GLuint min_filter=GL_LINEAR, GLuint mag_filter=GL_LINEAR);

Texture2DData make_y(double y0, double yf, int type, int width, int height,
                     GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                     GLuint min_filter=GL_LINEAR, GLuint mag_filter=GL_LINEAR);

Texture2DData ddx(const Texture2DData &f, struct Grad2DParams params);

Texture2DData ddy(const Texture2DData &f, struct Grad2DParams params);

Texture2DData fft(const Texture2DData &x);

Texture2DData ifft(const Texture2DData &x);

Texture2DData fft_omp(const Texture2DData &x);

Texture2DData ifft_omp(const Texture2DData &x);

Texture2DData fftshift(const Texture2DData &x);

Texture2DData fftshift(const Texture2DData &x, int type);

Texture2DData cast_to(int type,
                      const Texture2DData &x,
                      Channel c0, Channel c1, Channel c2, Channel c3,
                      const Texture2DData &y,
                      Channel d0, Channel d1, Channel d2, Channel d3);

Texture2DData roll(const Texture2DData &x, struct Vec2 v);

Texture2DData roll(const Texture2DData &x, double a, double b);

Texture2DData substitute(const Texture2DData &x,
                         double old_val, double new_val);

Texture2DData zeroes(int type, int width, int height,
                     bool generate_mipmap=true,
                     GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                     GLuint min_filter=GL_LINEAR, 
                     GLuint mag_filter=GL_LINEAR);

#endif
