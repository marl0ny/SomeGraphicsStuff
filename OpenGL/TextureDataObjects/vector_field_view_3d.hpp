#include "gl_wrappers/gl_wrappers.h"
#include "texture_data.hpp"

class VectorFieldView3D {
    frame_id frame;
    GLuint program;
    struct Vec4 *vertices;
    struct IVec2 view_dimensions;
    struct IVec3 vector_dimensions;
    public:
    VectorFieldView3D(IVec2 view_dimensions, IVec3 vector_dimensions);
    Texture2DData render(const Texture2DData &col, // Texture to dictate colour of each vector
                         const Texture2DData &vec, // Contains actual vector field texture data
                         float view_scale, // Scale the view
                         struct Vec4 rotation // Rotate the view.
                        ) const;

    ~VectorFieldView3D() {
        free(vertices);
    }
    VectorFieldView3D(const VectorFieldView3D &v); // TODO!
    VectorFieldView3D& operator=(const VectorFieldView3D &v); // TODO!

};
