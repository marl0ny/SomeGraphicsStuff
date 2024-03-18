#include "gl_wrappers/gl_wrappers.h"
#include "texture_data.hpp"

class VectorFieldView2D {
    frame_id frame;
    GLuint program;
    struct Vec4 *vertices;
    int vector_width, vector_height;
    int view_width, view_height;
    public:
    VectorFieldView2D(int view_width, int view_height,
                      int vector_width, int vector_height);
    Texture2DData render(const Texture2DData &col, // Texture to dictate colour of each vector
                         const Texture2DData &vec // Contains actual vector field texture data
                        ) const;

    ~VectorFieldView2D() {
        free(vertices);
    }
    VectorFieldView2D(const VectorFieldView2D &v); // TODO!
    VectorFieldView2D& operator=(const VectorFieldView2D &v); // TODO!

};
