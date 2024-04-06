#include "vector_field_view_2d.hpp"


static const char ATTRIBUTE_NAME[] = "inputData";

VectorFieldView2D::VectorFieldView2D(
    int view_width, int view_height,
    int vector_width, int vector_height) {
    this->vector_width = vector_width;
    this->vector_height = vector_height;
    this->view_width = view_width;
    this->view_height = view_height;
    this->program
         = make_program("./shaders/vec-render/view.vert",
          "./shaders/vec-render/view.frag");
    struct TextureParams tex_params = {
        .format=GL_RGBA32F,
        .width=this->view_width, .height=this->view_height,
        .generate_mipmap=1,
        .wrap_s=GL_CLAMP_TO_EDGE, .wrap_t=GL_CLAMP_TO_EDGE,
        .mag_filter=GL_LINEAR, .min_filter=GL_LINEAR,
    };
    int sizeof_vertices = 2*sizeof(struct Vec4)*vector_width*vector_height;
    this->vertices = (struct Vec4 *)malloc(sizeof_vertices);
    int w = vector_width;
    int h = vector_height;
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
}

Texture2DData VectorFieldView2D::render(const Texture2DData &col,
                                        const Texture2DData &vec
                                        ) const {
    // std::fprintf(stdout, "%d,%d,%g\n", view_width, view_height, view_scale);
    // std::fprintf(stdout, "%g,%g,%g,%g\n", 
    //              rotation.x, rotation.y, rotation.z, rotation.w);
    glViewport(0, 0, this->view_width, this->view_height);
    GLboolean depth_test_prev_enabled = glIsEnabled(GL_DEPTH_TEST);
    if (!depth_test_prev_enabled)
        glEnable(GL_DEPTH_TEST);
    bind_frame(this->frame, this->program);
    struct VertexParam vertex_params[2] = {
        {.name=(char *)&ATTRIBUTE_NAME[0], .size=4, .type=GL_FLOAT, 
         .normalized=GL_FALSE, .stride=4*sizeof(float), .offset=0},
    };
    set_vertex_attributes(vertex_params, 1);
    set_float_uniform("viewScale", 1.0);
    set_float_uniform("vecScale", 1.0);
    /* set_ivec2_uniform("texelDimensions2D", 
                      this->pixel_width*this->pixel_length,
                      this->pixel_height);*/
    set_sampler2D_uniform("vecTex", vec.frame);
    set_sampler2D_uniform("colTex", col.frame);
    glDrawArrays(GL_LINES, 0, 
                 2*this->vector_width*this->vector_height);
    unbind();
    /* glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    glClear(GL_STENCIL_BUFFER_BIT);*/
    if (!depth_test_prev_enabled)
        glDisable(GL_DEPTH_TEST);
    Texture2DData t = Texture2DData(FLOAT4, this->view_width, this->view_height,
    true, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
    tex_copy(t.frame, this->frame);
    return t;
}