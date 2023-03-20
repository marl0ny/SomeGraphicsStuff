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
    NONE=-1, X=0, Y=1, Z=2, W=3,
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
    friend Texture2DData cos(const Texture2DData &x);
    friend Texture2DData sin(const Texture2DData &x);
    friend Texture2DData exp(const Texture2DData &x);
    friend Texture2DData make_x(double x0, double xf,
                                int type, int width, int height,
                                GLuint wrap_s, GLuint wrap_t,
                                GLuint min_filter, GLuint mag_filter);
    friend Texture2DData make_y(double x0, double xf,
                                int type, int width, int height,
                                GLuint wrap_s, GLuint wrap_t,
                                GLuint min_filter, GLuint mag_filter);
    friend class DrawTexture2DData;
    Texture2DData(int type, frame_id frame,
                  const struct TextureParams &tex_params);
public:
    Texture2DData(int type, int width, int height,
                  bool generate_mipmap=true,
                  GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                  GLuint min_filter=GL_LINEAR, GLuint mag_filter=GL_LINEAR);
    Texture2DData(const Texture2DData &x);
    Texture2DData& operator=(const Texture2DData &x);
    Texture2DData(Texture2DData &&x);
    Texture2DData &operator=(Texture2DData &&x);
    void paste_to_quad(frame_id quad);
    void debug_print_state();
    void set_as_sampler2D_uniform(const char *name);
    Texture2DData const laplacian(struct Laplacian2DParams params);
    Texture2DData cast_to(int type, Channel c0);
    Texture2DData cast_to(int type, Channel c0, Channel c1);
    Texture2DData cast_to(int type, Channel c0, Channel c1, Channel c2);
    Texture2DData cast_to(int type,
                          Channel c0, Channel c1, Channel c2, Channel c3);
    ~Texture2DData() {
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

Texture2DData cos(const Texture2DData &x);

Texture2DData sin(const Texture2DData &x);

Texture2DData exp(const Texture2DData &x);

Texture2DData make_x(double x0, double xf, int type, int width, int height,
                     GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                     GLuint min_filter=GL_LINEAR, GLuint mag_filter=GL_LINEAR);

Texture2DData make_y(double x0, double xf, int type, int width, int height,
                     GLuint wrap_s=GL_REPEAT, GLuint wrap_t=GL_REPEAT,
                     GLuint min_filter=GL_LINEAR, GLuint mag_filter=GL_LINEAR);

#endif
