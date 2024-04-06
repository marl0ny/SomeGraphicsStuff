#include "vector_field_view_3d.hpp"
#include "quaternions.hpp"


static const char ATTRIBUTE_NAME[] = "inputData";

VectorFieldView3D::VectorFieldView3D(IVec2 view_dimensions, 
                                     IVec3 vector_dimensions) {
    for (int i = 0; i < 3; i++) {
        if (i < 3)
            this->view_dimensions.ind[i] = view_dimensions.ind[i];
        this->vector_dimensions.ind[i] = vector_dimensions.ind[i];
    }
    int w = vector_dimensions.x*vector_dimensions.z;
    int h = vector_dimensions.y;
    this->program
         = make_program("./shaders/vec-render/view3d.vert",
          "./shaders/vec-render/view.frag");
    struct TextureParams tex_params = {
        .format=GL_RGBA32F,
        .width=this->view_dimensions.x,
        .height=this->view_dimensions.y,
        .generate_mipmap=1,
        .wrap_s=GL_CLAMP_TO_EDGE, .wrap_t=GL_CLAMP_TO_EDGE,
        .min_filter=GL_NEAREST, .mag_filter=GL_NEAREST,
    };
    int sizeof_vertices = 2*sizeof(struct Vec4)*w*h;
    this->vertices = (struct Vec4 *)malloc(sizeof_vertices);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            this->vertices[2*(i*w + j)].x = ((float)j + 0.5)/(float)w;
            this->vertices[2*(i*w + j)].y = ((float)i + 0.5)/(float)h;
            this->vertices[2*(i*w + j)].z = 0.0;
            this->vertices[2*(i*w + j)].w = 0.0;
            this->vertices[2*(i*w + j) + 1].x = ((float)j + 0.5)/(float)w;
            this->vertices[2*(i*w + j) + 1].y = ((float)i + 0.5)/(float)h;
            this->vertices[2*(i*w + j) + 1].z = 0.0;
            this->vertices[2*(i*w + j) + 1].w = 0.5;
        }
    }
    this->frame = new_frame(&tex_params,
         (float *)vertices, sizeof_vertices, NULL, -1);
    free(vertices);
}

Texture2DData VectorFieldView3D::render(const Texture2DData &col,
                                        const Texture2DData &vec,
                                        float view_scale, Quaternion rotation
                                        ) const {
    // std::fprintf(stdout, "%d,%d,%g\n", view_width, view_height, view_scale);
    // std::fprintf(stdout, "%g,%g,%g,%g\n", 
    //              rotation.x, rotation.y, rotation.z, rotation.w);
    glViewport(0, 0, view_dimensions.x, view_dimensions.y);
    GLboolean depth_test_prev_enabled = glIsEnabled(GL_DEPTH_TEST);
    if (!depth_test_prev_enabled)
        glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    bind_frame(this->frame, this->program);
    struct VertexParam vertex_params[2] = {
        {.name=(char *)&ATTRIBUTE_NAME[0], .size=4, .type=GL_FLOAT, 
         .normalized=GL_FALSE, .stride=4*sizeof(float), .offset=0},
    };
    set_vertex_attributes(vertex_params, 1);
    set_float_uniform("viewScale", view_scale);
    set_vec4_uniform("rotation", 
                     rotation.x, rotation.y,
                     rotation.z, rotation.w);
    set_float_uniform("vecScale", 1.0);
    /* set_ivec2_uniform("texelDimensions2D", 
                      this->pixel_width*this->pixel_length,
                      this->pixel_height);*/
    set_ivec3_uniform("texelDimensions3D",
                      this->vector_dimensions.x, 
                      this->vector_dimensions.y, 
                      this->vector_dimensions.z);
    set_sampler2D_uniform("vecTex", vec.frame);
    set_sampler2D_uniform("colTex", col.frame);
    glDrawArrays(GL_LINES, 0, 
                 2*this->vector_dimensions.x
                 *this->vector_dimensions.y*this->vector_dimensions.z);
    unbind();
    // glClear(GL_COLOR_BUFFER_BIT);
    // glClear(GL_DEPTH_BUFFER_BIT);
    // glClear(GL_STENCIL_BUFFER_BIT);
    if (!depth_test_prev_enabled)
        glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    Texture2DData t = Texture2DData(FLOAT4, 
        this->view_dimensions.x, this->view_dimensions.y,
    true, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
    tex_copy(t.frame, this->frame);
    return t;
}